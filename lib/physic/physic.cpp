#include "physic.h"
#include "manchester.h"

static DigitalOut tx(p22);
static DigitalOut rx(p21);

Mail<bit_message_t, 10> messages_to_send;
char tick = 0;

int
write_message()
{
  DigitalOut led(LED4);

  while (1) {
    osEvent evt = messages_to_send.get();

    if (evt.status == osEventMail) {
      bit_message_t *tmp = (bit_message_t *)evt.value.p;
      bit_message_t message = *tmp;
      messages_to_send.free(tmp);

      int index = message.length - 1;
      while (index >= 0) {
        char current = message.message[index];
        int bitIndex = 7;

        while (bitIndex >= 0) {
          char manchester[2];
          bit_to_write(manchester, (current >> bitIndex) & 0x01);

          int manchesterCounter = 0;
          while (manchesterCounter < 2) {
            printf("\r\n");
            if (tick) {
              tick = 0;
              led = !led;
              tx = manchester[manchesterCounter];
              manchesterCounter++;
            }
          }
          bitIndex--;
        }
        index--;
      }
    }
  }
}
