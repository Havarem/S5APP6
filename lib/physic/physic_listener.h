#ifndef __S5APP6_PHYSIC_LISTENER_H
#define __S5APP6_PHYSIC_LISTENER_H

#include <mbed.h>
#include <rtos.h>

#define DEBUG_LISTENER "PHY_LIST"

void start_listener_th(void);

Thread * get_listener(void);

#endif
