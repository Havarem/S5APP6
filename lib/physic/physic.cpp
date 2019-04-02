#include "physic.h"
#include "mbed.h"
#include "manchester.h"

DigitalOut tx(p22);
DigitalOut tx_led(LED4);
DigitalOut led3(LED3);

Ticker sendBit_tick;
Thread write_th(osPriorityHigh, 1024);

Mail<trame_t, 10> messages_to_send;

void
write_message_routine(void)
{
  write();
}

void
start_write()
{
    sendBit_tick.attach_us(bit_ready_interupt, BIT_PERIOD);
    write_th.start(write_message_routine);
}

void
bit_ready_interupt(void)
{
  write_th.signal_set(0x1);
  led3 = !led3;
}


void
send_message_to_phy(trame_t &_trame)
{
    trame_t *trame = messages_to_send.alloc();
    if (trame != NULL) {
        trame->length = _trame.length;
        for(int i = 0; i < _trame.length; i++)
        {
            trame->message[i] = _trame.message[i];
        }

        messages_to_send.put(trame);
    }
    else
    {
        printf("Mailbox full\r\n");
    }

}

void
write(void)
{

    while (1) {
        osEvent evt = messages_to_send.get();
        //printf("New message\r\n");

        if (evt.status == osEventMail) {
            trame_t *tmp = (trame_t *)evt.value.p;
            trame_t message = *tmp;

            //printf("message length: %d\r\n", message.length);


            char full_message[160];
            for(int i = 0; i < message.length; i++)
            {
                //printf("0x%02x ", message.message[i]);
                char res[2];
                to_manchester(res, message.message[i]);
                full_message[i*2] = res[1];
                full_message[i*2 + 1] = res[0];
            }

            //printf("\r\nfull message \r\n");

            for(int i = 0; i < message.length * 2; i++)
            {
                int j = 7;
                while(j >= 0)
                {
                    Thread::signal_wait(0x1);
                    tx = (full_message[i] >> j) & 0x01;
                    tx_led = (full_message[i] >> j) & 0x01;
                    //printf("tick: %d\r\n", (full_message[i] >> j) & 0x01);
                    j--;
                }
                //printf("--------");
            }

            //RESET tx to 0 after message
            Thread::signal_wait(0x1);
            tx = 0;
            tx_led = 0;

            printf("Message done\r\n");
            messages_to_send.free(tmp);
            Thread::wait(3500);
        }
    }
}