#include "contiki.h"
#include "rpl.h"
#include "httpd-simple.h"
// #include "net/ipv6/simple-udp.h"
#include <stdio.h>
// #include "net/routing/routing.h"
// #include "net/netstack.h"

#include "../shared/defs.h"

#include "sys/log.h"
#define LOG_MODULE "Web sense"
#define LOG_LEVEL LOG_LEVEL_INFO
#define BUFFER_SIZE 50

// static char names[MAX_NUMBER_SOURCES][20];
// static int next_free_name = 0;

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
// Defined callback, that is setup in simple_udp_register
// static struct simple_udp_connection udp_conn;
// static void data_receiver(
//     struct simple_udp_connection *c,
//     const uip_ipaddr_t *sender_addr,
//     uint16_t sender_port,
//     const uip_ipaddr_t *receiver_addr,
//     uint16_t receiver_port,
//     const uint8_t *data,
//     uint16_t datalen)
// {
//   printf("received data at ROOT: %s\n", (char *)data);

//   // char temp_name[UIPLIB_IPV6_MAX_STR_LEN];
//   // uiplib_ipaddr_snprint(temp_name, sizeof(temp_name), sender_addr);
//   // // sprintf(ip_of_mote, "%s", remove_colon(ip_of_mote));

//   // bool fileExists = false;
//   // int index_of_name = -1;
//   // for(i = 0; i < MAX_NUMBER_AGGREGATORS; i++)
//   // {
//   //   if(strcmp(names[i], remove_colon(temp_name)) == 0)
//   //   {
//   //     fileExists = true;
//   //     index_of_name = i;
//   //     break;
//   //   }
//   // }

//   // if (!fileExists)
//   // {
//   //   snprintf(names[next_free_name], sizeof(temp_name), "%s", remove_colon(temp_name));
//   //   index_of_name = next_free_name;
//   //   next_free_name++;
//   // }

//   // int fd = cfs_open(names[index_of_name], CFS_WRITE | CFS_APPEND);
//   // if(fd < 0)
//   // {
//   //   printf("Error opening file \n");
//   // }
//   // else
//   // {
//   //   int e = cfs_write(fd, data, datalen);
//   //   if (e < 0)
//   //   {
//   //     printf("Errpr writing to file \n");
//   //   }
//   // }
  
//   // cfs_close(fd);
// }
/*---------------------------------------------------------------------------*/
// static struct simple_udp_connection event_conn;
// static void event_receiver(
//     struct simple_udp_connection *c,
//     const uip_ipaddr_t *sender_addr,
//     uint16_t sender_port,
//     const uip_ipaddr_t *receiver_addr,
//     uint16_t receiver_port,
//     const uint8_t *data,
//     uint16_t datalen)
// {
//     LOG_INFO("Root Received EVENT '%.*s' from ", datalen, (char *)data);
//     LOG_INFO_6ADDR(sender_addr);
//     LOG_INFO_("\n");
// }
/*---------------------------------------------------------------------------*/



PROCESS_THREAD(root, ev, data)
{
    // static int err = 1;
    
    PROCESS_BEGIN();
    /* Initialize DAG root */
    // NETSTACK_ROUTING.root_start();

    // /* Initialize UDP connection */
    // err = simple_udp_register(&udp_conn, ROOT_DATA_PORT, NULL, AGGR_ROOTDATA_PORT, data_receiver);
    // if (err == 0) {
    //   LOG_ERR("ERROR: Could not etablish data connection \n");
    // }
    
    // err = simple_udp_register(&event_conn, ROOT_EVENT_PORT, NULL, AGGR_ROOTEVENT_PORT, event_receiver);
    // if (err == 0) {
    //   LOG_ERR("ERROR: Could not etablish event connection \n");
    // }
    
    LOG_INFO("Root: Done initialzation v.3\n");


    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
//                              Webserver stuff                              //
/*---------------------------------------------------------------------------*/

static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  char buff[15];
  // int rfd = -1;

  PSOCK_BEGIN(&s->sout);

  sprintf(buff,"25,26");
  printf("send json to requester\n");

  // for (int i = 0; i < next_free_name; i++)
  // {
  //   rfd = cfs_open(names[i], CFS_READ);
  //   if(rfd < 0)
  //   {
  //     printf("ERROR: Opnening file with name: %s\n", names[i]);
  //   }
  //   else
  //   {
  //     int bytesRead = cfs_read(rfd,buff, BUFFER_SIZE)
  //     if (bytesRead < 0)
  //     {
  //       printf("ERROR: Reading file with name: %s\n", names[i]);
  //     }
  //   }
  //   cfs_close(rfd);
  //   SEND_STRING(&s->sout, buff);
  // }


  SEND_STRING(&s->sout, buff);

  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
PROCESS(webserver, "Web server server");
PROCESS_THREAD(webserver, ev, data)
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
PROCESS(root, "root");
PROCESS(webserver_starter, "web server");
AUTOSTART_PROCESSES(&root ,&webserver_starter);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(webserver_starter, ev, data)
{
  PROCESS_BEGIN();

  PROCESS_NAME(webserver);
  process_start(&webserver, NULL);

  LOG_INFO("web server started++++++\n");

  PROCESS_END();
}



