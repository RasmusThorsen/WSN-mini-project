#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "sys/energest.h"
#include "sensor.h"
#include "lib/random.h"
#include "node-id.h"

#include "../shared/defs.h"
#include "../shared/utility.h"

#include "sys/log.h"
#define LOG_MODULE "Source"
#define LOG_LEVEL LOG_LEVEL_INFO

static uip_ipaddr_t root_ip;
static struct simple_udp_connection sender_connection;
static struct simple_udp_connection root_connection;

PROCESS(source, "source");
// PROCESS(printenergy, "printenergy");

AUTOSTART_PROCESSES(&source);
/*---------------------------------------------------------------------------*/
// Called when receiving Aggregator IP
// static struct simple_udp_connection broadcast_connection;
static uip_ipaddr_t aggregator_ip;
static bool ipFlag = false;
static void receiver(struct simple_udp_connection *c,
                     const uip_ipaddr_t *sender_addr,
                     uint16_t sender_port,
                     const uip_ipaddr_t *receiver_addr,
                     uint16_t receiver_port,
                     const uint8_t *data,
                     uint16_t datalen)
{
    if (strncmp((char *)data, "B", 1) == 0)
    {
        LOG_INFO("Received IP... \n");
        aggregator_ip = *sender_addr;
        ipFlag = true;
    }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(source, ev, data)
{
    uip_ipaddr_t addr;

    static struct Sensor temp;
    static struct Sensor humid;

    static struct etimer broadcast_timer;
    static struct etimer send_timer;

    static int status_code_temp = 0;
    static int status_code_humid = 0;
    static char buffer[32];

    PROCESS_BEGIN();

    // SENSORS_ACTIVATE(sht11_sensor);

    // Node ID Only works with cooja - https://sourceforge.net/p/contiki/mailman/message/29073409/
    random_init(node_id);

    init_sensors();

    int err = simple_udp_register(&sender_connection, SOURCE_PORT, NULL, AGGR_PORT, receiver);
    if (err == 0)
    {
        LOG_ERR("ERROR: Could not etablish broadcast connection \n");
    }

    err = simple_udp_register(&root_connection, SOURCE_PORT, NULL, ROOT_PORT, NULL);
    if (err == 0)
    {
        LOG_ERR("ERROR: Could not etablish root connection \n");
    }

    etimer_set(&broadcast_timer, CLOCK_SECOND * 5);

    // Multicast until IP of some aggregator is received
    while (!ipFlag)
    {
        LOG_INFO("Looking for aggregator... \n");

        uip_create_linklocal_allnodes_mcast(&addr);
        simple_udp_sendto(&sender_connection, "B", 2, &addr);

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&broadcast_timer));
        etimer_reset(&broadcast_timer);
    }

    etimer_set(&send_timer, CLOCK_SECOND * 60);
    while (1)
    {
        status_code_temp = read_temperature(&temp);
        status_code_humid = read_humidity(&humid);

        if (status_code_temp == 0 && status_code_humid == 0)
        {
            // LOG_INFO("Data: %d.%02d,%d.%02d \n", temp.intergerValue, temp.decimal, humid.intergerValue, humid.decimal);
            if (temp.intergerValue > TEMP_TRESHOLD)
            {
                if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip)) {
                    snprintf(buffer, sizeof(buffer), "%d,%d", temp.intergerValue, humid.intergerValue);
                    simple_udp_sendto(&root_connection, buffer, strlen(buffer), &root_ip);
                } else {
                    LOG_ERR("ERROR: Cannot reach root \n");
                }
            }
            snprintf(buffer, sizeof(buffer), "D,%d.%02d,%d.%02d", temp.intergerValue, temp.decimal, humid.intergerValue, humid.decimal);
            simple_udp_sendto(&sender_connection, buffer, strlen(buffer), &aggregator_ip);
        }
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));
        etimer_reset(&send_timer);
    }

    PROCESS_END();
}

// Remember #include "contiki.h"
// Global static functions can be declared outside scope of process
// PROCESS_THREAD(printenergy, ev, data)
// {
//     static struct etimer print_timer;

//     PROCESS_BEGIN();
//     etimer_set(&print_timer, CLOCK_SECOND * 60); 

//     while(1) {
//         // energest_report();
//         LOG_INFO("Events sent: %d \n", events_send);
//         PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&print_timer));
//         etimer_reset(&print_timer);
//     } 

//     PROCESS_END();
// }