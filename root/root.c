#include "contiki.h"
#include "dev/sht11/sht11-sensor.h"
#include <math.h>
#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "rpl.h"
#include "httpd-simple.h"

#define CLIENT_PORT 1111
#define SERVER_PORT 2222

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

static struct simple_udp_connection udp_conn;

// Defined callback, that is setup in simple_udp_register
static void udp_rx_callback(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
    LOG_INFO("Received request '%.*s' from ", datalen, (char *)data);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
}

// Remember #include "contiki.h"
// Global static functions can be declared outside scope of process

PROCESS(root, "root");
PROCESS(webserver, "WebServer");
AUTOSTART_PROCESSES(&root, &webserver);


PROCESS_THREAD(root, ev, data)
{
    PROCESS_BEGIN();

    /* Initialize DAG root */
    NETSTACK_ROUTING.root_start();

    /* Initialize UDP connection */
    simple_udp_register(&udp_conn, SERVER_PORT, NULL,
                        CLIENT_PORT, udp_rx_callback);

    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
//                              Webserver stuff                              //
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  char buff[15];

  PSOCK_BEGIN(&s->sout);

  sprintf(buff,"25,26");
  LOG_INFO("Sending data");

  SEND_STRING(&s->sout, buff);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(webserver, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("WebServer Started\n");

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