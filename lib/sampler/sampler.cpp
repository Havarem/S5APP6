#include "sampler.h"

Semaphore sampler_tick;

/**
 * Different state of the sampler
 * */
typedef enum {
  WAITING = 0,
  RESET = 1,
  HIGH = 2,
  LOW = 3,
  SYNCHING_HIGH = 4,
  SYNCHING_LOW = 5,
  ERROR = 6
} SampleState;

const static char SampleStateName[7][8] = {
  { 'W', 'A', 'I', 'T', 'I', 'N', 'G', '\0' },
  { 'R', 'E', 'S', 'E', 'T', '\0' },
  { 'H', 'I', 'G', 'H', '\0' },
  { 'L', 'O', 'W', '\0' },
  { 'S', '-', 'H', 'I', 'G', 'H', '\0' },
  { 'S', '-', 'L', 'O', 'W', '\0' },
  { 'E', 'R', 'R', 'O', 'R' }
};

// Sampler Thread instance
static Thread sampler;
// Physic-layer thread reference
static Thread *physic;
// Sampler Ticker instance
static Ticker sampler_ticker;
// Signal to listen
static PinName rx;

static Timer period_timer;
// Sampler period in nano-second
static us_timestamp_t period;
// Sampler state-machine current state
static SampleState state;

/**
 * ISR for the Sampler Ticker.
 * */
static void
sampler_tick_routine(void)
{
  sampler_tick.release();
}

/**
 * Routine called by the sampler thread.
 * */
static void
sampler_th(void)
{
  us_timestamp_t thigh = 0;
  us_timestamp_t tlow = 0;

  int value;
  int ceh = 0;
  int cel = 0;

  DigitalIn irx(rx);

  while(1) {
    sampler_tick.wait(osWaitForever);
    value = irx;

    switch(state){
    case WAITING:
      if (value == 1) {
        state = SYNCHING_HIGH;
        period_timer.reset();
        period_timer.start();
      }
      break;
    case RESET:
      period = MAX_SAMPLING_PERIOD;
      state = WAITING;
      break;
    case HIGH:
      if (value == 1) {
        ceh++;
      } else {
        if (ceh > 0) {
          state = ERROR;
          break;
        }
        state = LOW;
        cel = 1;
      }
      break;
    case LOW:
      if (value == 0) {
        cel++;
      } else {
        if (cel > 0) {
          state = ERROR;
          break;
        }
        state = HIGH;
        ceh = 1;
      }
      break;
    case SYNCHING_HIGH:
      if (value == 0) {
        period_timer.stop();
        thigh = period_timer.read_high_resolution_us();
        period_timer.reset();
        period_timer.start();
        state = SYNCHING_LOW;
      }
      break;

    case SYNCHING_LOW:
      if (value == 1) {
        period_timer.stop();
        tlow = period_timer.read_high_resolution_us();

        int diff = thigh > tlow ? thigh - tlow : tlow - thigh;

        if (diff < 75) {
          state = HIGH;
          period = (thigh + tlow) >> 2;
          sampler_ticker.detach();
          sampler_ticker.attach_us(sampler_tick_routine, period);
          ceh = 1;
        } else {
          state = ERROR;
        }
      }
      break;

    case ERROR:
      if (value == 0) {
        state = WAITING;
      }
      break;
    }

    if (ceh == 2 || cel == 2) {
      ceh = cel = 0;
      physic->flags_set(value == 0 ? 0x01 : 0x02);
    }
  }
}

void
start_sampler_th(PinName _rx, Thread * _phy)
{
  rx = _rx;
  period = MAX_SAMPLING_PERIOD;
  state = WAITING;
  physic = _phy;
  sampler.start(sampler_th);
  sampler_ticker.attach_us(sampler_tick_routine, period);
  sampler.set_priority(osPriorityRealtime);
}

us_timestamp_t
get_current_period(void)
{
  return period;
}