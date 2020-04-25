#include <stdlib.h> // needed for strtok
#include <stdio.h> // snprintf and printf

#include "contiki.h"

#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/uiplib.h"

#include "os/storage/cfs/cfs.h"
#include "os/lib/heapmem.h"

#include "../shared/defs.h"
#include "../shared/utility.h"

#define BUFFER_SIZE 50
#define TIME_BETWEEN_AGGREGATION 10

static uip_ipaddr_t root_ip;
static struct simple_udp_connection root_connection;

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
static struct simple_udp_connection source_event_connection;
static struct simple_udp_connection root_event_connection;
static void event_receiver(struct simple_udp_connection *c,
                           const uip_ipaddr_t *sender_addr,
                           uint16_t sender_port,
                           const uip_ipaddr_t *receiver_addr,
                           uint16_t receiver_port,
                           const uint8_t *data,
                           uint16_t datalen)
{
    printf("Received event\n");
    if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip))
    {
        char transmit_buffer[32];
        snprintf(transmit_buffer, sizeof(transmit_buffer), "%s", (char *)data);
        simple_udp_sendto(&root_event_connection, transmit_buffer, strlen(transmit_buffer), &root_ip);
    }
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
    printf("received data at AGGR: %s\n", (char *)data);

    // Remove colons from IP so it can be used as a valid filename
    char temp_name[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(temp_name, sizeof(temp_name), sender_addr);

    bool nameExists = false;
    int index_of_name = -1;
    int i;
    for (i = 0; i < MAX_NUMBER_SOURCES; i++)
    {
        if (strcmp(names[i], remove_colon(temp_name)) == 0)
        {
            nameExists = true;
            index_of_name = i;
            break;
        }
    }

    if (!nameExists)
    {
        snprintf(names[next_free_name], sizeof(temp_name), remove_colon(temp_name));
        index_of_name = next_free_name;
        next_free_name++;
    }

    int fd = cfs_open(names[index_of_name], CFS_WRITE | CFS_APPEND);
    if (fd < 0)
    {
        printf("Error opening file \n");
    }
    else
    {
        int e = cfs_write(fd, data, datalen);
        if (e < 0)
        {
            printf("Error writing to file \n");
        }
    }

    cfs_close(fd);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aggregator, ev, data)
{
    static struct etimer readFileTimer;
    static char buf[BUFFER_SIZE];
    static uint8_t values[BUFFER_SIZE];
    static int i, j, u, rfd, bytesRead;
    static int acc_temp = 0, n_temp = 0;
    static char *token;
    static char transmit_buffer[32];

    PROCESS_BEGIN();

    simple_udp_register(&broadcast_connection, AGGR_BROADCAST_PORT, NULL, SOURCE_PORT, broadcast_receiver);
    simple_udp_register(&data_connection, AGGR_DATA_PORT, NULL, SOURCE_PORT, data_receiver);
    simple_udp_register(&source_event_connection, AGGR_EVENT_PORT, NULL, SOURCE_EVENT_PORT, event_receiver);
    simple_udp_register(&root_event_connection, AGGR_EVENT_PORT, NULL, ROOT_EVENT_PORT, NULL);
    simple_udp_register(&root_connection, AGGR_PORT, NULL, ROOT_PORT, NULL);

    etimer_set(&readFileTimer, CLOCK_SECOND * TIME_BETWEEN_AGGREGATION);

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&readFileTimer));

        j = 0;
        for (i = 0; i < next_free_name; i++)
        {
            rfd = cfs_open(names[i], CFS_READ);
            if (rfd < 0)
            {
                printf("ERROR: Opnening file with name: %s\n", names[i]);
            }
            else
            {
                bytesRead = cfs_read(rfd, buf, BUFFER_SIZE);
                if (bytesRead < 0)
                {
                    printf("ERROR: Reading file with name: %s\n", names[i]);
                }
            }
            cfs_close(rfd);

            token = strtok(buf, ",");
            values[0] = simple_atoi(token);
            while (token != NULL)
            {
                j++;
                if (j >= BUFFER_SIZE)
                {
                    break;
                }

                token = strtok(NULL, ",");
                values[j] = simple_atoi(token);
            }

            // values will properly contain "raw" values, so everyother element would for instance be temp.
            for (u = 0; u <= j; u++)
            {
                acc_temp += values[u];
                n_temp++;
            }
        }

        if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip))
        {
            printf("transmit to root \n");
            snprintf(transmit_buffer, sizeof(transmit_buffer), "%d", acc_temp / n_temp);
            simple_udp_sendto(&root_connection, transmit_buffer, strlen(transmit_buffer), &root_ip);
        }

        // acc_temp = 0;
        // n_temp = 0;
        // int q;
        // for(q = 0; q < next_free_name; q++) {
        //     memset(names[q], 0, 20);
        //     cfs_remove(names[q]);
        // }
        // next_free_name = 0;
        etimer_reset(&readFileTimer);
    }

    PROCESS_END();
}
