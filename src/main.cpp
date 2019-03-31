#include <mbed.h>
#include <rtos.h>

#include "Logger.h"
#include "sampler_man.h"
#include "physic_listener.h"

DigitalOut led(LED1);

int
main()
{
  Logger::getLogger().logDebug("MAIN THREAD", "Welcome my friends");

  osThreadId id = ThisThread::get_id();
  osThreadSetPriority(id, osPriorityRealtime1);

  start_listener_th();
  start_sampler_th(p21, get_listener());

  osThreadSetPriority(id, osPriorityIdle);

  ThisThread::sleep_for(osWaitForever);
}

/*void
physical_routine(void)
{
  int count = 7;
  uint8_t current = 0x00;
  while (1) {
    ThisThread::flags_wait_any_for(0x03, osWaitForever, false);
    uint32_t flag = ThisThread::flags_get();
    uint8_t value = 0x00;


    printf("Entering switch: %d\r\n", flag);
    switch (flag) {
      case 0x01:
        value = 0x00;
        break;

      case 0x02:
        value = 0x01;
        break;

      case 0x03:
        value = 0x01;
        count = 6;
        break;

      default:
        current = 0;
        count = 7;
        goto end_of_loop;
    }

    current = (value << count) | current;
    printf("(%d, %d, %d, %d)\r\n", flag, value, current, count);

    if (count == 0) {
      printf("0x%02x \r\n", current);
      count = 8;
      current = 0x00;
    }
    count--;

end_of_loop:
    ThisThread::flags_clear(0x07);
  }
}*/

/*Ticker sendBit;
Thread sendToWrite;
Thread doWrite;

#define FREQ 1000

float frequency = FREQ / 1000.0f;

void
bit_ready_interupt(void)
{
  tick = 1;
}

void
send_to_write_routine(void)
{
  while (1) {
    bit_message_t *message = messages_to_send.alloc();
    if (message != NULL) {
      //printf("Adding content\r\n");
      message->message[0] = (uint8_t)'A';
      message->message[1] = (uint8_t)'Z';
      message->message[2] = (uint8_t)'T';
      message->length = 3;
      messages_to_send.put(message);
    } else {
      printf("Mailbox full\r\n");
    }
    ThisThread::sleep_for(FREQ * 40);
  }
}

int main() {
  printf("Frequency: %f\r\n", frequency);
  sendBit.attach(bit_ready_interupt, frequency);

  sendToWrite.start(send_to_write_routine);
  doWrite.start(write_message);

  while(1) {
    if (tick) {
      printf("Tick\r\n");
      doWrite.flags_set(0x01);
    }
  }
}
*/