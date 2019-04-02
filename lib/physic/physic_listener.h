#ifndef __S5APP6_PHYSIC_LISTENER_H
#define __S5APP6_PHYSIC_LISTENER_H

#include <mbed.h>
#include <rtos.h>

#include "Logger.h"

#define DEBUG_LISTENER "PHY_LIST"

void start_listener_th(Mail<uint8_t, 100> *);

Thread * get_listener(void);

#endif
