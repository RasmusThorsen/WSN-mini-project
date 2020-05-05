
#include "contiki.h"
#include "rpl.h"
#include "httpd-simple.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/uiplib.h"

// #include "os/storage/cfs/cfs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../shared/defs.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Web Sense"
#define LOG_LEVEL LOG_LEVEL_INFO

static uip_ipaddr_t processor_ip;
static struct simple_udp_connection processor_conn;
static bool data_received = false;
static bool ipFlag = false;
static char buff[50];

static void receiver(struct simple_udp_connection *c,
                     const uip_ipaddr_t *sender_addr,
                     uint16_t sender_port,
                     const uip_ipaddr_t *receiver_addr,
                     uint16_t receiver_port,
                     const uint8_t *data,
                     uint16_t datalen)
{
  // LOG_INFO("Initdata '%s' \n", (char*)data);
  if (strncmp((char *)data, "W", 1) == 0)
  {
      LOG_INFO("Received IP... \n");
      processor_ip = *sender_addr;
      ipFlag = true;
  }
  else 
  {
      snprintf(buff, sizeof(buff), "%s", data);
      LOG_INFO("Webserver Received Data '%s' \n", buff);
      data_received = true;
      simple_udp_sendto(&processor_conn, "A", 2, sender_addr);
  }
}


/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  // static struct etimer waiting_for_data;

  PSOCK_BEGIN(&s->sout);

  simple_udp_sendto(&processor_conn,"D", 2, &processor_ip);
  PSOCK_WAIT_UNTIL(&s->sout, data_received == true);

  data_received = false;

  SEND_STRING(&s->sout, buff);

  PSOCK_END(&s->sout);

  // To make sure the buffer is shorter than the data we receive
  strtok(buff, ",");
}
/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web Sense server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}
/*---------------------------------------------------------------------------*/
/* Declare and auto-start this file's process */
PROCESS(web_sense, "Web Sense");
PROCESS(requester, "Start");
AUTOSTART_PROCESSES(&web_sense, &requester);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(web_sense, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_NAME(webserver_nogui_process);
  process_start(&webserver_nogui_process, NULL);

  LOG_INFO("Web Sense started\n");

  PROCESS_END();
}

PROCESS_THREAD(requester, ev, data)
{
  static struct etimer broadcast_timer;
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  int err = simple_udp_register(&processor_conn, WEBSERVER_PORT, NULL, PROCESSOR_PORT, receiver);
  if (err == 0)
  {
    LOG_ERR("ERROR: Could not etablish broadcast connection \n");
  }

  etimer_set(&broadcast_timer, CLOCK_SECOND * 2);

  // Multicast until IP of some aggregator is received
  while (!ipFlag)
  {
    LOG_INFO("Webserver looking for processor... \n");

    uip_create_linklocal_allnodes_mcast(&addr);
    simple_udp_sendto(&processor_conn, "W", 2, &addr);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&broadcast_timer));
    etimer_reset(&broadcast_timer);
  }

  PROCESS_END();
}