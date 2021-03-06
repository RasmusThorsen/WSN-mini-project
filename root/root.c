#include "contiki.h"

#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include <string.h>
#include "net/netstack.h"
#include "net/routing/routing.h"
#include "net/ipv6/uiplib.h"

#include "../shared/defs.h"
#include "../shared/utility.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO

static int received_event = 0;

/*---------------------------------------------------------------------------*/
static struct simple_udp_connection processor_conn;
static uip_ipaddr_t processor_ip;
static bool ipFlag = false;

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
  LOG_INFO("Root Received Data '%.*s' from ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  // Forward data when received
  char ip_of_mote[UIPLIB_IPV6_MAX_STR_LEN];
  uiplib_ipaddr_snprint(ip_of_mote, sizeof(ip_of_mote), sender_addr);

  char buffer[50];

  strcpy(buffer, trim(ip_of_mote, ':'));
  strcat(buffer, ",");
  strcat(buffer, (char*)data);

  simple_udp_sendto(&processor_conn, buffer, strlen(buffer), &processor_ip);
}

static void processor_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
  if (strncmp((char*)data, "P", 1) == 0)
  {
    LOG_INFO("Received IP... \n");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
    processor_ip = *sender_addr;
    ipFlag = true;
  }
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

/* Declare and auto-start this file's process */
PROCESS(contiki_ng_br, "Contiki-NG Border Router");
PROCESS(root, "Root");
PROCESS(printenergy, "printenergy");
AUTOSTART_PROCESSES(&contiki_ng_br, &root, &printenergy);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contiki_ng_br, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Contiki-NG Border Router started\n");
 
  PROCESS_END();
}

PROCESS_THREAD(root, ev, data)
{
  uip_ipaddr_t addr;
  static struct etimer broadcast_timer;

  PROCESS_BEGIN();
  /* Initialize DAG root (This should now be started though RPL)*/

  /* Initialize UDP connection */
  int err = simple_udp_register(&processor_conn, ROOT_PORT, NULL, PROCESSOR_PORT, processor_receiver);
  if (err == 0) {
    LOG_ERR("ERROR: Could not etablish data connection \n");
  }

  err = simple_udp_register(&aggr_connection, ROOT_PORT, NULL, AGGR_PORT, data_receiver);
  if(err == 0) {
      LOG_ERR("ERROR: Could not etablish data connection \n");
  }

  err = simple_udp_register(&root_connection, ROOT_PORT, NULL, SOURCE_PORT, event_receiver);
  if(err == 0) {
      LOG_ERR("ERROR: Could not etablish data connection \n");
  }

  etimer_set(&broadcast_timer, CLOCK_SECOND * 2);

    // Multicast until IP of some aggregator is received
    while (!ipFlag)
    {
        LOG_INFO("Root looking for processor... \n");

        uip_create_linklocal_allnodes_mcast(&addr);
        simple_udp_sendto(&processor_conn, "P", 2, &addr);

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&broadcast_timer));
        etimer_reset(&broadcast_timer);
    }

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

