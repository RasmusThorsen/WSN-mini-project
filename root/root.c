#include "contiki.h"
#include "dev/sht11/sht11-sensor.h"
#include <math.h>
#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include "net/routing/routing.h"
#include "net/netstack.h"

#include "../shared/defs.h"
#include "../shared/utility.h"

#include "sys/log.h"
#define LOG_MODULE "Root"
#define LOG_LEVEL LOG_LEVEL_INFO

static int received_event = 0;

/*---------------------------------------------------------------------------*/
PROCESS(root, "root");
PROCESS(printenergy, "printenergy");
AUTOSTART_PROCESSES(&root, &printenergy);
/*---------------------------------------------------------------------------*/
// Defined callback, that is setup in simple_udp_register
static struct simple_udp_connection aggr_connection;
static void data_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
    LOG_INFO("Root Received request '%.*s' from ", datalen, (char *)data);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
// Defined callback, that is setup in simple_udp_register
static struct simple_udp_connection root_connection;
static void event_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
    received_event++;
    LOG_INFO("Root Received EVENT '%.*s' from ", datalen, (char *)data);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(root, ev, data)
{
    PROCESS_BEGIN();
    /* Initialize DAG root */
    NETSTACK_ROUTING.root_start();

    /* Initialize UDP connection */
    int err = simple_udp_register(&aggr_connection, ROOT_PORT, NULL, AGGR_PORT, data_receiver);
    if(err == 0) {
        LOG_ERR("ERROR: Could not etablish data connection \n");
    }

    err = simple_udp_register(&root_connection, ROOT_PORT, NULL, SOURCE_PORT, event_receiver);
    if(err == 0) {
        LOG_ERR("ERROR: Could not etablish data connection \n");
    }
    LOG_INFO("Root: Done initialzation \n");

    PROCESS_END();
}

PROCESS_THREAD(printenergy, ev, data)
{
    static struct etimer print_timer;

    PROCESS_BEGIN();
    etimer_set(&print_timer, CLOCK_SECOND * 60); 

    while(1) {
        // energest_report();
        LOG_INFO("Received events: %d \n", received_event);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&print_timer));
        etimer_reset(&print_timer);
    } 

    PROCESS_END();
}
