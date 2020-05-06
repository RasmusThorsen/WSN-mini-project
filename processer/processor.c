#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"

#include "net/ipv6/uiplib.h"

#include "../shared/defs.h"
#include "../shared/utility.h"

#include "sys/log.h"
#define LOG_MODULE "processor"
#define LOG_LEVEL LOG_LEVEL_INFO

static char names[MAX_NUMBER_AGGREGATORS][50];
static int next_free_name = 0;
char buffer_removeip[20];

char* removeIp(char* input)
{

    uint8_t i, commaPos;
    bool comma = false;
    for(i = 0; i < 20; i++)
    {
        if(comma)
        {
            buffer_removeip[i-commaPos-1] = input[i];
        }
        else if (input[i] == ',') {
            comma = true;
            commaPos = i;
        }
    }
    LOG_INFO("Buffer: %s\n", buffer_removeip);
    return buffer_removeip;

}

PROCESS(processor, "processor");
AUTOSTART_PROCESSES(&processor);
/*---------------------------------------------------------------------------*/
// Called when receiving Aggregator IP
// static struct simple_udp_connection broadcast_connection;
static struct simple_udp_connection udp_conn;
static struct simple_udp_connection webserver_conn;
static void data_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
    if(strncmp((char*)data, "P", 1) == 0) {
        LOG_INFO("Sending IP to ROOT \n");
        simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
    }
    else
    {
        LOG_INFO("Processor Received Data '%.*s' from ", datalen, (char *)data);
        LOG_INFO_6ADDR(sender_addr);
        LOG_INFO_("\n");

        char ip_of_mote[UIPLIB_IPV6_MAX_STR_LEN];
        char* input = (char *)data;
        uint8_t i;
        char buffer[20];

        for (i = 0; i<strlen(input); i++)
        {
            if (input[i]!=(int)',') {
                ip_of_mote[i]=input[i];
            }
            else
                break;
        }

        bool fileExists = false;
        int index_of_name = -1;

        for(i = 0; i < MAX_NUMBER_AGGREGATORS; i++)
        {
            if(strncmp(names[i], ip_of_mote, strlen(ip_of_mote)) == 0)
            {
                fileExists = true;
                index_of_name = i;
                LOG_INFO("MATCH! \n");
                break;
            }
        }

        if (!fileExists)
        {
            LOG_INFO("Processor2 Received Data '%.*s' from \n", datalen, (char *)data);

            snprintf(buffer, sizeof(ip_of_mote), "%s;", data);
            index_of_name = next_free_name;
            strcat(names[index_of_name], buffer);
            next_free_name++;
        }
        else
        {
            snprintf(buffer, sizeof(ip_of_mote), "%s;", removeIp((char *)data));
            strcat(names[index_of_name], buffer);
        }

        LOG_INFO("\n DATA: %s \n", names[index_of_name]);
    }
}

static uint8_t next_item_to_ship = 0;
static void webserver_handler(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
    if (strncmp((char*)data, "W",1) == 0) {
        LOG_INFO("Sending IP to webserver \n");
        simple_udp_sendto(&webserver_conn, data, datalen, sender_addr);
    }
    else if(strncmp((char*)data, "D",1) == 0)
    {
        LOG_INFO("WEBSERVER SENT A REQUEST\n names: %s\n", names[next_item_to_ship]);
        simple_udp_sendto(&webserver_conn, names[next_item_to_ship], sizeof(names[next_item_to_ship]), sender_addr);
        LOG_INFO("After sent names: %.*s\n", sizeof(names[next_item_to_ship]),names[next_item_to_ship]);
    }
    else if(strncmp((char*)data, "A",1) == 0)
    {
        LOG_INFO("ACK before clear data \n names: %s\n", names[next_item_to_ship]);

        strtok(names[next_item_to_ship], ",");
        strcat(names[next_item_to_ship], ",");            
        LOG_INFO("ACK clear data \n names: %s\n", names[next_item_to_ship]);

        next_item_to_ship++;
        if(next_item_to_ship >= next_free_name)
            next_item_to_ship = 0;
    }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(processor, ev, data)
{


    PROCESS_BEGIN();

    int err = simple_udp_register(&udp_conn, PROCESSOR_PORT, NULL, ROOT_PORT, data_receiver);
    if (err == 0)
    {
        LOG_ERR("ERROR: Could not establish broadcast connection \n");
    }

    err = simple_udp_register(&webserver_conn, PROCESSOR_PORT, NULL, WEBSERVER_PORT, webserver_handler);
    if(err == 0)
    {
        LOG_ERR("ERROR: Could not establish connection to webserver \n");
    }


    PROCESS_END();
}
