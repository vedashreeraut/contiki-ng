
#include "contiki.h"
#include "contiki-net.h"
#include "sys/etimer.h"
#include "random.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>

#define TEMP_THRESHOLD 38
#define HEART_THRESHOLD 100
#define OXYGEN_THRESHOLD 90
#define UDP_PORT 1234

static struct uip_udp_conn *udp_conn;

PROCESS(alert_node_process, "Healthcare Alert Node");
AUTOSTART_PROCESSES(&alert_node_process);

PROCESS_THREAD(alert_node_process, ev, data)
{
  static struct etimer timer;
  static int temp, heart_rate, spo2;
  static char msg[100];
  static uint32_t timestamp;
  static int alert_count = 0;

  PROCESS_BEGIN();

  udp_conn = udp_new(NULL, UIP_HTONS(UDP_PORT), NULL);
  udp_bind(udp_conn, UIP_HTONS(UDP_PORT));

  printf("Alert node started, sending on port %d\n", UDP_PORT);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    timestamp = clock_time();

    temp = 36 + (random_rand() % 5);         // 36–40°C
    heart_rate = 70 + (random_rand() % 50);  // 70–120 bpm
    spo2 = 88 + (random_rand() % 13);        // 88–100 %

    printf("Vitals: Temp = %d°C, HR = %d bpm, SpO2 = %d%%\n", temp, heart_rate, spo2);

    if (temp > TEMP_THRESHOLD || heart_rate > HEART_THRESHOLD || spo2 < OXYGEN_THRESHOLD) {
      const char *severity;
      if (temp > TEMP_THRESHOLD + 1 || heart_rate > HEART_THRESHOLD + 10 || spo2 < OXYGEN_THRESHOLD - 5) {
        severity = "CRITICAL";
      } else {
        severity = "MODERATE";
      }

      snprintf(msg, sizeof(msg),
               "ALERT #%d [%s] @%lu: Temp=%d HR=%d SpO2=%d",
               ++alert_count, severity, timestamp, temp, heart_rate, spo2);

      leds_on(LEDS_RED);

      uip_create_linklocal_allnodes_mcast(&udp_conn->ripaddr);
      uip_udp_packet_send(udp_conn, msg, strlen(msg));
      uip_create_unspecified(&udp_conn->ripaddr);

      printf("📡 Sent alert: %s\n", msg);
    } else {
      leds_off(LEDS_RED);
    }
  }

  PROCESS_END();
}
