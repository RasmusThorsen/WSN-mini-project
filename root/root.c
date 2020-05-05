/*
 * Copyright (c) 201, RISE SICS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"

#include "net/ipv6/simple-udp.h"
#include <stdio.h>
#include <string.h>
#include "net/netstack.h"
#include "net/routing/routing.h"
// #include "os/storage/cfs/cfs.h"
#include "net/ipv6/uiplib.h"

#include "../shared/defs.h"
// #include "../shared/utility.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO

char* trim(char* input, char trim)
{
    int i,j;
    char *output=input;
    for (i = 0, j = 0; i<strlen(input); i++,j++)
    {
        if (input[i]!=(int)trim)
            output[j]=input[i];
        else
            j--;
    }
    output[j]=0;
    return output;
}

static struct simple_udp_connection processor_conn;
static uip_ipaddr_t processor_ip;
static bool ipFlag = false;

// Defined callback, that is setup in simple_udp_register
static struct simple_udp_connection aggr_connection;
static void data_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
  LOG_INFO("Root Received Data '%.*s' from ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  // Forward data when received
  char ip_of_mote[UIPLIB_IPV6_MAX_STR_LEN];
  uiplib_ipaddr_snprint(ip_of_mote, sizeof(ip_of_mote), sender_addr);

  char buffer[50];

  strcpy(buffer, trim(ip_of_mote, ':'));
  strcat(buffer, ",");
  strcat(buffer, (char*)data);

  simple_udp_sendto(&processor_conn, buffer, strlen(buffer), &processor_ip);
}

static void processor_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
  if (strncmp((char*)data, "P", 1) == 0)
  {
    LOG_INFO("Received IP... \n");
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
    processor_ip = *sender_addr;
    ipFlag = true;
  }
}
/*---------------------------------------------------------------------------*/
// Defined callback, that is setup in simple_udp_register
static struct simple_udp_connection root_connection;
static void event_receiver(
    struct simple_udp_connection *c,
    const uip_ipaddr_t *sender_addr,
    uint16_t sender_port,
    const uip_ipaddr_t *receiver_addr,
    uint16_t receiver_port,
    const uint8_t *data,
    uint16_t datalen)
{
    LOG_INFO("Root Received EVENT '%.*s' from ", datalen, (char *)data);
    LOG_INFO_6ADDR(sender_addr);
    LOG_INFO_("\n");
}

/* Declare and auto-start this file's process */
PROCESS(contiki_ng_br, "Contiki-NG Border Router");
PROCESS(root, "Root");
AUTOSTART_PROCESSES(&contiki_ng_br, &root);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contiki_ng_br, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Contiki-NG Border Router started\n");
 
  PROCESS_END();
}

PROCESS_THREAD(root, ev, data)
{
  uip_ipaddr_t addr;
  static struct etimer broadcast_timer;

  PROCESS_BEGIN();
  /* Initialize DAG root (This should now be started though RPL)*/
  // NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  int err = simple_udp_register(&processor_conn, ROOT_PORT, NULL, PROCESSOR_PORT, processor_receiver);
  if (err == 0) {
    LOG_ERR("ERROR: Could not etablish data connection \n");
  }

  err = simple_udp_register(&aggr_connection, ROOT_PORT, NULL, AGGR_PORT, data_receiver);
  if(err == 0) {
      LOG_ERR("ERROR: Could not etablish data connection \n");
  }

  err = simple_udp_register(&root_connection, ROOT_PORT, NULL, SOURCE_PORT, event_receiver);
  if(err == 0) {
      LOG_ERR("ERROR: Could not etablish data connection \n");
  }

  etimer_set(&broadcast_timer, CLOCK_SECOND * 2);

    // Multicast until IP of some aggregator is received
    while (!ipFlag)
    {
        LOG_INFO("Root looking for processor... \n");

        uip_create_linklocal_allnodes_mcast(&addr);
        simple_udp_sendto(&processor_conn, "P", 2, &addr);

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&broadcast_timer));
        etimer_reset(&broadcast_timer);
    }

  LOG_INFO("Root: Done initialzation \n");


  PROCESS_END();
}