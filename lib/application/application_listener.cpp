#include "application_listener.h"

Mail<uint8_t, 100> link_bytes_pool;

static Thread linker(osPriorityNormal, 4096);

typedef enum {
  APP_WAITING = 0,
  APP_PREAMBULE = 1,
  APP_START = 2,
  FLAG = 3,
  LENGTH = 4,
  DATA = 5,
  CRC_1 = 6,
  CRC_2 = 7,
  APP_END = 8,
  APP_ERROR = 9
} ApplicationState;

static ApplicationState state;
static char current_message[74];
static uint8_t message_length;
static uint8_t cursor;
static unsigned short current_crc;

static inline int
check_crc(void)
{
  unsigned short crc = crc16(current_message, message_length);
  return crc == current_crc ? 1 : 0;
}

static inline void
app_state_machine(uint8_t current)
{
  printf("state machine (%d)\r\n", state);
  switch (state) {
  case APP_WAITING:
    if (current == 0x55) {
      state = APP_PREAMBULE;
    }
    break;

  case APP_PREAMBULE:
    if (current == 0x7E) {
      state = APP_START;
    }
    break;

  case APP_START:
    state = FLAG;
    break;

  case FLAG:
    message_length = current;
    cursor = 0;
    state = LENGTH;
    break;

  case LENGTH:
    if (message_length == 0) {
      current_crc = current_crc | current;
      current_message[0] = '\0';
      state = CRC_1;
    } else {
      current_message[0] = current;
      cursor++;
      state = DATA;
    }
    break;

  case DATA:
    if (cursor == message_length) {
      current_crc = current_crc | current;
      current_message[cursor] = '\0';
      state = CRC_1;
    } else {
      current_message[cursor] = current;
      cursor++;
    }
    break;

  case CRC_1:
    current_crc = current_crc | (current << 8);
    state = CRC_2;
    break;

  case CRC_2:
    printf("CRC2\r\n");
    if (check_crc() && current == 0x7E) {
      state = APP_WAITING;
      printf("Message: %s\r\n", current_message);
    } else {
      state = APP_ERROR;
      printf("Error CRC\r\n");
      printf("Message: %s\r\n", current_message);
    }
    break;

  case APP_END:
    printf("Message: %s\r\n", current_message);
    state = APP_WAITING;
    break;

  case APP_ERROR:
    state = APP_WAITING;
    break;
  }
}

static void
application_th(void)
{
  uint8_t current_byte = 0x00;
  while (1) {
    osEvent event = link_bytes_pool.get();
    if (event.status == osEventMail) {
      //Logger::getLogger().logDebug(DEBUG_APP, "Got mail");
      uint8_t *byte = (uint8_t *)event.value.p;
      current_byte = *byte;
      link_bytes_pool.free(byte);

      app_state_machine(current_byte);
    }
  }
}

void
start_application_th(void)
{
  state = APP_WAITING;
  cursor = 0;
  message_length = 0;
  linker.start(application_th);
}
