#include <mbed.h>
#include <rtos.h>

#include "physic.h"

DigitalOut led(LED1);

Ticker sendBit;
Thread sendToWrite;
Thread doWrite;

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
      strcpy(message->message, "AAA");
      message->length = 3;
      messages_to_send.put(message);
      ThisThread::sleep_for(3300);
    } else {
      printf("Mailbox full\r\n");
    }
  }
}

void
write_message_routine(void)
{
  write_message();
}

int main() {

  sendBit.attach(bit_ready_interupt, 0.1);

  sendToWrite.start(send_to_write_routine);
  doWrite.start(write_message_routine);

  while(1) {
    ThisThread::sleep_for(200);
    led = !led;
  }
}