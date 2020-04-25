#include "contiki.h"
#include "dev/sht11/sht11-sensor.h"
#include <math.h>
#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include "net/routing/routing.h"
#include "net/netstack.h"

#include "../shared/defs.h"

#include "sys/log.h"
#define LOG_MODULE "Root"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
PROCESS(root, "root");
AUTOSTART_PROCESSES(&root);
/*---------------------------------------------------------------------------*/
// Defined callback, that is setup in simple_udp_register
static struct simple_udp_connection udp_conn;
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
static struct simple_udp_connection event_conn;
static void event_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
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
    simple_udp_register(&udp_conn, ROOT_PORT, NULL, AGGR_PORT, data_receiver);
    simple_udp_register(&event_conn, ROOT_EVENT_PORT, NULL, AGGR_EVENT_PORT, event_receiver);
    LOG_INFO("Root: Done initialzation \n");

    PROCESS_END();
}
