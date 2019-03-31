#include "physic_listener.h"

static Thread listener;

static uint8_t current_byte;
static uint8_t position;

/**
 * Routine called by the sampler thread.
 * */
static void
listener_th(void)
{
  while (1) {
    //printf("Listening flags from samplers\r\n");
    ThisThread::flags_wait_any_for(0xFF, osWaitForever, false);
    uint32_t flags = ThisThread::flags_get();
    //printf("Flag: (%u)\r\n", flags);

    switch (flags) {
      case 1:
        if (position == 0) {
          // Notify the linker about the current byte received;

          printf("current_byte: 0x%02x\r\n", current_byte);
          current_byte = 0x00;
          position = 7;
        } else {
          position--;
        }
        break;

      case 2:
        if (position == 0) {
          current_byte |= 0x01;
          // Notify the linker about the current byte received;

          printf("current_byte: 0x%02x\r\n", current_byte);
          current_byte = 0x00;
          position = 7;
        } else {
          current_byte |= (0x01 << position);
          position--;
        }
        break;

      case 4:
        // We have the "010" sequence at top MSB of the byt
        current_byte = 0x40;
        position = 4;
        break;

      case 8:   // passthrough
      default:
        printf("Default\r\n");
        current_byte = 0x00;
        position = 7;
        break;
    }
    ThisThread::flags_clear(flags);
  }
}

void
start_listener_th(void)
{
  current_byte = 0x00;
  position = 7;
  listener.start(listener_th);
  listener.set_priority(osPriorityAboveNormal);
}

Thread *
get_listener(void)
{
  return &listener;
}