#include "contiki.h"
#include "dev/sht11/sht11-sensor.h"
#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "net/nullnet/nullnet.h"
#include "net/ipv6/uip.h"

#define CLIENT_PORT 1111
#define SERVER_PORT 2222

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(source, "source");
AUTOSTART_PROCESSES(&source);

// static struct simple_udp_connection udp_conn; 

// static unsigned long
// to_seconds(uint64_t time)
// {
//   return (unsigned long)(time / ENERGEST_SECOND);
// }

// void
// energest_report(void) 
// {
//     energest_flush();
//     printf("\nEnergest:\n");
//     printf(" CPU          %lu LPM      %lu DEEP LPM %lu  Total time %lu\n",
//         to_seconds(energest_type_time(ENERGEST_TYPE_CPU)),
//         to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
//         to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
//         to_seconds(ENERGEST_GET_TOTAL_TIME()));
//     printf(" Radio LISTEN %lu TRANSMIT %lu OFF      %lu \n",
//         to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
//         to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
//         to_seconds(ENERGEST_GET_TOTAL_TIME() - energest_type_time(ENERGEST_TYPE_TRANSMIT) - energest_type_time(ENERGEST_TYPE_LISTEN)));
// }
static struct simple_udp_connection broadcast_connection;
PROCESS_THREAD(source, ev, data)
{
    // static uip_ipaddr_t dest_ipaddr; 
    // static float temp = 0;
    // static char buffer[32];
    static struct etimer timer;
    uip_ipaddr_t addr;
    
    PROCESS_BEGIN();

    simple_udp_register(&broadcast_connection, CLIENT_PORT, NULL, CLIENT_PORT, NULL);

    etimer_set(&timer, CLOCK_SECOND * 2); 
    while(1) {
        LOG_INFO("Source broadcasting.. \n");
        uip_create_linklocal_allnodes_mcast(&addr);
        simple_udp_sendto(&broadcast_connection, "Test", 4, &addr);


        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);
    }
    // SENSORS_ACTIVATE(sht11_sensor); 

    // simple_udp_register(&udp_conn, CLIENT_PORT, NULL, SERVER_PORT, NULL); 

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

    // // Deactive sensors 
    SENSORS_DEACTIVATE(sht11_sensor);

    PROCESS_END();
}
