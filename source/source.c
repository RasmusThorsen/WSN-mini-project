#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "sensor.h"
#include "lib/random.h"
#include "node-id.h"

#include "../shared/defs.h"

#include "sys/log.h"
#define LOG_MODULE "Source"
#define LOG_LEVEL LOG_LEVEL_INFO

static struct simple_udp_connection sender_connection;
static struct simple_udp_connection event_connection;

PROCESS(source, "source");
AUTOSTART_PROCESSES(&source);
/*---------------------------------------------------------------------------*/
// Called when receiving Aggregator IP
static struct simple_udp_connection broadcast_connection;
static uip_ipaddr_t aggregator_ip;
static bool ipFlag = false;
static void receive_aggregator_ip(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
    aggregator_ip = *sender_addr;
    ipFlag = true;
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(source, ev, data)
{
    static uip_ipaddr_t dest_ipaddr; 
    static int status_code_temp = 0;
    static int status_code_humid = 0;
    static char buffer[32];
    static struct etimer timer;
    static struct Sensor temp;
    static struct Sensor humid;
    static struct etimer broadcast_timer;
    static struct etimer send_timer;
    uip_ipaddr_t addr;
    
    static int counter = 0;
    static char buf[32];

    PROCESS_BEGIN();

    SENSORS_ACTIVATE(sht11_sensor); 
    
    // Node ID Only works with cooja - https://sourceforge.net/p/contiki/mailman/message/29073409/
    random_init(node_id);

    simple_udp_register(&broadcast_connection, SOURCE_BROADCAST_PORT, NULL, AGGR_BROADCAST_PORT, receive_aggregator_ip);
    etimer_set(&broadcast_timer, CLOCK_SECOND * 2); 

    // Multicast until IP of some aggregator is received
    while(!ipFlag) {
        uip_create_linklocal_allnodes_mcast(&addr);
        simple_udp_sendto(&broadcast_connection, "0", 2, &addr);

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&broadcast_timer));
        etimer_reset(&broadcast_timer);
    }

    simple_udp_register(&sender_connection, SOURCE_DATA_PORT, NULL, AGGR_DATA_PORT, NULL); // TODO wait for ack
    simple_udp_register(&event_connection, SOURCE_EVENT_PORT, NULL, AGGR_EVENT_PORT, NULL);
    etimer_set(&send_timer, CLOCK_SECOND * 2); 

    while(1) {
        status_code_temp = read_temperature(&temp);
        status_code_humid = read_humidity(&humid);

        if(status_code_temp == 0 && status_code_humid == 0) 
        {
            snprintf(buffer, sizeof(buffer), "temp: %d.%02d, humid: %d.%02d", temp.intergerValue, temp.decimal, humid.intergerValue, humid.decimal);
            LOG_INFO("Sender data: %d.%02d,%d.%02d \n", temp.intergerValue, temp.decimal, humid.intergerValue, humid.decimal);
            simple_udp_sendto(&sender_connection, buffer, strlen(buffer), &aggregator_ip);
        }

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));
        etimer_reset(&send_timer);
    }

    PROCESS_END();
}

