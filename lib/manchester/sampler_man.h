#ifndef __S5APP6_SAMPLER_H
#define __S5APP6_SAMPLER_H

/**
 * Session 5 APP6 Sampler module
 *
 * The sampler module will need a ticker that will be high enough to detect
 * the zero and one. That will give us the ability to measure the frequency
 * (or period) for one bit.
 *
 * The rules are simple:
 *  - It will detect the period between two changes, from a 0 to 1, back to a 0
 *  - The adjustement can only increase, never decrease
 *  -
 * */

#include <mbed.h>
#include <rtos.h>

#include "Logger.h"

#define DEBUG_SAMPLER "SAMPLER"

// The maximum frequency to start with, in nano-seconds.
#define MAX_SAMPLING_PERIOD 500 // 0.5ms

// This synchronize the thread with the ticker.
extern Semaphore sampler_tick;

/**
 * This starts the sampler thread.
 * */
void start_sampler_th(Thread *);

/**
 * Return the current period in which the system is sampling.
 *
 * @return us_timestamp_t A 64-bits representation in nano-second.
 * */
us_timestamp_t get_current_period(void);

#endif
