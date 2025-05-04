#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "sys/etimer.h"
#include <stdio.h>

#define TEMP_THRESHOLD 38
#define HEART_THRESHOLD 100

PROCESS(alert_node_process, "Healthcare Alert Node");
AUTOSTART_PROCESSES(&alert_node_process);

static void recv_callback(struct broadcast_conn *c, const linkaddr_t *from, const uint8_t *data, uint16_t len) {
  printf("Alert Received from %02x:%02x: %s\n", from->u8[0], from->u8[1], data);
}

static const struct broadcast_callbacks bc_callbacks = {recv_callback};
static struct broadcast_conn bc;

PROCESS_THREAD(alert_node_process, ev, data) {
  static struct etimer timer;
  static int temp, heart_rate;
  char msg[50];

  PROCESS_EXITHANDLER(broadcast_close(&bc);)
  PROCESS_BEGIN();

  broadcast_open(&bc, 129, &bc_callbacks);

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 5);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Simulate data
    temp = 36 + (random_rand() % 5);         // 36–40°C
    heart_rate = 70 + (random_rand() % 50);  // 70–120 bpm

    printf("Measured Temp: %d, HR: %d\n", temp, heart_rate);

    // Check for alert
    if(temp > TEMP_THRESHOLD || heart_rate > HEART_THRESHOLD) {
      sprintf(msg, "ALERT! Temp:%d HR:%d", temp, heart_rate);
      packetbuf_copyfrom(msg, strlen(msg) + 1);
      broadcast_send(&bc);
      printf("Sent alert: %s\n", msg);
    }
  }

  PROCESS_END();
}
