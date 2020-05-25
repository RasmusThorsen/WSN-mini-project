#include <stdlib.h> // needed for strtok
#include <stdio.h>  // snprintf and printf
#include <string.h> // strncmp

#include "contiki.h"

#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/uiplib.h"

#include "os/storage/cfs/cfs.h"

#include "sys/log.h"
#define LOG_MODULE "Aggregator"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "../shared/defs.h"
#include "../shared/utility.h"

#define BUFFER_SIZE 200
#define TIME_BETWEEN_AGGREGATION 500

static uip_ipaddr_t root_ip;
static struct simple_udp_connection root_connection;

static char names[MAX_NUMBER_SOURCES][20];
static int next_free_name = 0;

/*---------------------------------------------------------------------------*/
PROCESS(aggregator, "aggregator");
AUTOSTART_PROCESSES(&aggregator);
/*---------------------------------------------------------------------------*/
// Handles sources that sends data
static struct simple_udp_connection source_connection;
static void data_receiver(struct simple_udp_connection *c,
                          const uip_ipaddr_t *sender_addr,
                          uint16_t sender_port,
                          const uip_ipaddr_t *receiver_addr,
                          uint16_t receiver_port,
                          const uint8_t *data,
                          uint16_t datalen)
{
    if(strncmp((char *)data, "B", 1) == 0) {
        LOG_INFO("Sending IP \n");
        simple_udp_sendto(&source_connection, data, datalen, sender_addr);
    } else {
        // Remove colons from IP so it can be used as a valid filename
        char temp_name[UIPLIB_IPV6_MAX_STR_LEN];
        uiplib_ipaddr_snprint(temp_name, sizeof(temp_name), sender_addr);
        bool nameExists = false;
        int index_of_name = -1;
        int i;
        for (i = 0; i < MAX_NUMBER_SOURCES; i++)
        {
            if (strcmp(names[i], trim(temp_name, ':')) == 0)
            {
                nameExists = true;
                index_of_name = i;
                break;
            }
        }

        if (!nameExists)
        {
            snprintf(names[next_free_name], sizeof(temp_name), "%s", trim(temp_name, ':'));
            index_of_name = next_free_name;
            next_free_name++;
        }

        int fd = cfs_open(names[index_of_name], CFS_WRITE | CFS_APPEND);
        if (fd < 0)
        {
            LOG_ERR("ERROR: Error opening file with name: %s \n", names[index_of_name]);
        }
        else
        {
            int e = cfs_write(fd, data, datalen);
            if (e < 0)
            {
                LOG_ERR("ERROR: Error writing to file with name: %s \n", names[index_of_name]);
            }
        }

        cfs_close(fd);
    }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aggregator, ev, data)
{
    static struct etimer readFileTimer;
    static char buf[BUFFER_SIZE];
    static double values[BUFFER_SIZE];
    static int i, j, u, rfd, bytesRead;
    static int acc_temp = 0, n_temp = 0;
    static int acc_hum = 0, n_hum = 0;
    static char *token;
    static char transmit_buffer[20];

    PROCESS_BEGIN();

    int err = simple_udp_register(&source_connection, AGGR_PORT, NULL, SOURCE_PORT, data_receiver);
    if(err == 0) {
        LOG_ERR("ERROR: Could not etablish data connection \n");
    }
    
    err = simple_udp_register(&root_connection, AGGR_PORT, NULL, ROOT_PORT, NULL);
    if(err == 0) {
        LOG_ERR("ERROR: Could not etablish root connection \n");
    }

    etimer_set(&readFileTimer, CLOCK_SECOND * TIME_BETWEEN_AGGREGATION);

    while (1)
    {
        LOG_INFO("Aggregating \n");
        j = 0;
        for (i = 0; i < next_free_name; i++)
        {
            rfd = cfs_open(names[i], CFS_READ);
            if (rfd < 0)
            {
                LOG_ERR("ERROR: Opnening file with name: %s\n", names[i]);
            }
            else
            {
                bytesRead = cfs_read(rfd, buf, BUFFER_SIZE);
                if (bytesRead < 0)
                {
                    LOG_ERR("ERROR: Reading file with name: %s\n", names[i]);
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
                values[j] = (int)simple_atof(token);
            }

            // skip first value because it is the type: event (E) or data (D)
            for (u = 1; u <= j; u++)
            {
                if(u % 2 == 0) {
                    acc_hum += values[u];
                    n_hum++;
                } else {
                    acc_temp += values[u];
                    n_temp++;
                }
            }
        }
        // LOG_INFO("Acc_Temp: %d, N_Temp: %d, Acc_Hum: %d, N_Hum: %d \n", acc_temp, n_temp, acc_hum, n_hum);
        // LOG_INFO("Data recevied: %d \n", data_recevied);
        if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&root_ip) && n_hum != 0 && n_temp != 0)
        {
            snprintf(transmit_buffer, sizeof(transmit_buffer), "%d,%d", (int)acc_temp / n_temp, (int)acc_hum / n_hum);
            simple_udp_sendto(&root_connection, transmit_buffer, strlen(transmit_buffer), &root_ip);

            acc_temp = 0; 
            n_temp = 0;
            acc_hum = 0;
            n_hum = 0;
            int q;
            for(q = 0; q < next_free_name; q++) {
                cfs_remove(names[q]);
                memset(names[q], 0, 20);
            }
            next_free_name = 0;
        } else {
            LOG_ERR("ERROR: Root not reachable or n-values equal 0: N Values are: %d and %d \n", n_temp, n_hum);
        }
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&readFileTimer));
        etimer_reset(&readFileTimer);
    }

    PROCESS_END();
}
