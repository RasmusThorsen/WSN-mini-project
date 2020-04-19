#include "contiki.h"
#include <stdlib.h>

#include "net/ipv6/simple-udp.h"
// #include "net/routing/routing.h"
// #include "net/netstack.h"
#include "net/ipv6/uiplib.h"

#include "os/storage/cfs/cfs.h"
// #include "os/lib/heapmem.h"

#include "../shared/defs.h"
#include "../shared/utility.h"

#include "sys/log.h"
#define LOG_MODULE "Aggregator"
#define LOG_LEVEL LOG_LEVEL_INFO

// static struct uip_ipaddr_t root_ip;
static char names[MAX_NUMBER_SOURCES][20];
static int next_free_name = 0;

/*---------------------------------------------------------------------------*/
PROCESS(aggregator, "aggregator");
AUTOSTART_PROCESSES(&aggregator);
/*---------------------------------------------------------------------------*/
// Handles broadcasting sources
static struct simple_udp_connection broadcast_connection;
static void broadcast_receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
    // Ack broadcaster so it receives aggregator IP through callback
    simple_udp_sendto(&broadcast_connection, data, datalen, sender_addr);
}
/*---------------------------------------------------------------------------*/
// Handles sources that sends data
static struct simple_udp_connection data_connection;
static void data_receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
    // Remove colons from IP so it can be used as a valid filename
    char temp_name[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(temp_name, sizeof(temp_name), sender_addr);

    bool nameExists = false;
    int index_of_name = -1;
    int i;
    for(i = 0; i < MAX_NUMBER_SOURCES; i++) {
        if(strcmp(names[i], remove_colon(temp_name)) == 0) {
            nameExists = true;
            index_of_name = i;
            break;
        }
    }

    if(!nameExists) {
        snprintf(names[next_free_name], sizeof(temp_name), remove_colon(temp_name));
        index_of_name = next_free_name;
        next_free_name++;
    }

    int fd = cfs_open(names[index_of_name], CFS_WRITE | CFS_APPEND);
    if(fd < 0) {
        LOG_ERR("Error opening file \n");
    } else {
        int e = cfs_write(fd, data, datalen);
        if(e < 0) {
            LOG_ERR("Error writing to file \n");
        }
    }

    cfs_close(fd);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(aggregator, ev, data)
{
    static struct etimer readFileTimer;
    static char buf[100];
    static int values[100];
    static int i;
    static int j = 0;
    static char * token;
    static int rfd;
    static int bytesRead;
    // int* const buf_ptr = heapmem_alloc((size_t)10);
    // int* const value_ptr = heapmem_alloc((size_t)10);
    // static int* value_index = 0;
    
    // static unsigned int accumulated_temp = 0;
    // static unsigned int n_temp = 0;
    // static unsigned int accumulated_hum = 0;
    // static unsigned int n_hum = 0;

    PROCESS_BEGIN();

    // char str[8];


    // int j;

    simple_udp_register(&broadcast_connection, AGGR_BROADCAST_PORT, NULL, SOURCE_PORT, broadcast_receiver);
    simple_udp_register(&data_connection, AGGR_DATA_PORT, NULL, SOURCE_PORT, data_receiver);

    etimer_set(&readFileTimer, CLOCK_SECOND * 2);

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&readFileTimer));
 
        for(i = 0; i < next_free_name; i++) {
            rfd = cfs_open(names[i], CFS_READ);
            if(rfd < 0) {
                LOG_ERR("ERROR: Opnening file with name: %s\n", names[i]);
            } else {
                bytesRead = cfs_read(rfd, buf, 100);
                if(bytesRead < 0) {
                    LOG_ERR("ERROR: Reading file with name: %s\n", names[i]);
                }
            }
            cfs_close(rfd); 

            token = strtok(buf, ",");
            values[j] = atoi(token);
            while(token != NULL) {
                j++;
                token = strtok(NULL, ",");
                values[j] = atoi(token);
            }

            LOG_INFO("value 0: %d \n", values[0]);
        }

        etimer_reset(&readFileTimer);
    }

    // if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip)) {
    //     // TODO: Reach out for root node with aggregated data on interval
    // }
    // heapmem_free(buf_ptr);
    // heapmem_free(value_ptr);
    PROCESS_END();
}
