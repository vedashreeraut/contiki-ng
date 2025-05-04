#include "contiki.h"
#include "contiki-net.h"
#include "sys/etimer.h"
#include "random.h"
#include <stdio.h>
#include <string.h>

#define TEMP_THRESHOLD 38
#define HEART_THRESHOLD 100
#define UDP_PORT 1234

static struct uip_udp_conn *udp_conn;

PROCESS(alert_node_process, "Healthcare Alert Node");
AUTOSTART_PROCESSES(&alert_node_process);

PROCESS_THREAD(alert_node_process, ev, data)
{
  static struct etimer timer;
  static int temp, heart_rate;
  static char msg[60];

  PROCESS_BEGIN();

  // Create a UDP connection to a broadcast address
  udp_conn = udp_new(NULL, UIP_HTONS(UDP_PORT), NULL);
  udp_bind(udp_conn, UIP_HTONS(UDP_PORT));

  printf("Alert node started, sending on port %d\n", UDP_PORT);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Simulate sensor data
    temp = 36 + (random_rand() % 5);         // 36–40°C
    heart_rate = 70 + (random_rand() % 50);  // 70–120 bpm

    printf("Measured Temp: %d, HR: %d\n", temp, heart_rate);

    if(temp > TEMP_THRESHOLD || heart_rate > HEART_THRESHOLD) {
      snprintf(msg, sizeof(msg), "ALERT! Temp:%d HR:%d", temp, heart_rate);
      uip_create_linklocal_allnodes_mcast(&udp_conn->ripaddr);
      uip_udp_packet_send(udp_conn, msg, strlen(msg));
      uip_create_unspecified(&udp_conn->ripaddr); // Reset address
      printf("Sent alert: %s\n", msg);
    }
  }

  PROCESS_END();
}
