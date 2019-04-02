#include "application_listener.h"

Mail<uint8_t, 100> link_bytes_pool;

static Thread linker(osPriorityNormal, 4096);

typedef enum {
  WAITING = 0,

} ApplicationState;

static void
application_th(void)
{
  while (1) {
    osEvent event = link_bytes_pool.get();
    if (event.status == osEventMail) {
      Logger::getLogger().logDebug(DEBUG_APP, "Got mail");
      uint8_t *byte = (uint8_t *)event.value.p;
      printf("0x%02x\r\n", *byte);
      link_bytes_pool.free(byte);
    }
  }
}

void
start_application_th(void)
{
  linker.start(application_th);
}
