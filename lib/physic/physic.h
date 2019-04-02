#ifndef __S5APP6_PHYSIC_H
#define __S5APP6_PHYSIC_H

#include "mbed.h"
#include "rtos.h"

#include "trame.h"
#include "manchester.h"

#define BIT_PERIOD 200000

extern Thread write_th;


void write(void);

void send_message_to_phy(trame_t &);

void bit_ready_interupt(void);

void start_write();



#endif