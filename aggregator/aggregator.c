#include <math.h>
#include <stdio.h>

#include "contiki.h"
#include "dev/sht11/sht11-sensor.h"

#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"

#define CLIENT_PORT 1111
#define SERVER_PORT 2222
#define ROOT_PORT 3333

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

// static struct simple_udp_connection udp_conn;
// static struct simple_udp_connection udp_conn2;
// static uip_ipaddr_t dest_ipaddr; 
// static uip_ipaddr_t dest_ipaddr2; 

// Defined callback, that is setup in simple_udp_register
// static void udp_rx_callback(
//     struct simple_udp_connection *c,
//     const uip_ipaddr_t *sender_addr,
//     uint16_t sender_port,
//     const uip_ipaddr_t *receiver_addr,
//     uint16_t receiver_port,
//     const uint8_t *data,
//     uint16_t datalen)
// {
//     LOG_INFO("Aggregator received request '%.*s' from ", datalen, (char *)data);
//     LOG_INFO_6ADDR(sender_addr);
//     LOG_INFO_("\n");
//     static char buffer[32];
//     if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr2)) {
//         snprintf(buffer, datalen, "AGGR %d", 2);
//         LOG_INFO("AGGR Sender data: %d \n", (int)2);
//         simple_udp_sendto(&udp_conn2, buffer, strlen(buffer), &dest_ipaddr2);
//     } else {
//         LOG_INFO("Not reachable yet \n");
//     }
// }
static struct simple_udp_connection broadcast_connection;
static void receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
}
// Remember #include "contiki.h"
// Global static functions can be declared outside scope of process
PROCESS(aggregator, "aggregator");
AUTOSTART_PROCESSES(&aggregator);
PROCESS_THREAD(aggregator, ev, data)
{
    // static char buffer[32];
    // static struct etimer timer;
    // static float temp = 0;
    PROCESS_BEGIN();
    // LOG_INFO("AGGR Init start \n");
    /* Initialize DAG root */
    // NETSTACK_ROUTING.root_start();
    simple_udp_register(&broadcast_connection, CLIENT_PORT, NULL, CLIENT_PORT, receiver);
    /* Initialize UDP connection */
    // simple_udp_register(&udp_conn, SERVER_PORT, NULL, CLIENT_PORT, udp_rx_callback);

    // simple_udp_register(&udp_conn2, SERVER_PORT, NULL, ROOT_PORT, NULL);
    // while(1) {
    //     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        
    //     float temp_raw = sht11_sensor.value(SHT11_SENSOR_TEMP);
    //     if(temp_raw != -1) {
    //         temp = ((0.01*temp_raw) - 39.60);
    //     } 

    //     if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    //         snprintf(buffer, sizeof(buffer), "temp: %d", (int)temp);
    //         LOG_INFO("Sender data: %d \n", (int)temp);
    //         simple_udp_sendto(&udp_conn, buffer, strlen(buffer), &dest_ipaddr);
    //     } else {
    //         LOG_INFO("Not reachable yet \n");
    //     }

    //     etimer_reset(&timer);
    // } 

    PROCESS_END();
}
