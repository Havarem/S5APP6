#include "sampler_man.h"

Semaphore sampler_tick;

/**
 * Sampler state
 * */
typedef enum {
  SYNCHING = 0,
  READY = 1,
  ERROR = 2
} SampleState;

const static char SampleStateName[3][9] = {
  { 'S', 'Y', 'N', 'C', 'H', 'I', 'N', 'G', '\0' },
  { 'R', 'E', 'A', 'D', 'Y', '\0' },
  { 'E', 'R', 'R', 'O', 'R', '\0' }
};

/**
 * Different states in the synching pathway
 * */
typedef enum {
  S_WAITING = 0,
  S_SYNCHING_HIGH = 1,
  S_SYNCHING_LOW = 2
} SampleSynchingState;

/**
 *
 * */
typedef enum {
  R_INIT = 0,
  R_HIGH_START = 1,
  R_HIGH_END = 2,
  R_LOW_START = 3,
  R_LOW_END = 4,
  R_DRIFT_HIGH = 5,
  R_DRIFT_LOW = 6
} SampleReadyState;

const static char ReadyStateName[7][7] = {
  { 'I', 'N', 'I', 'T', '\0' },
  { 'H', 'S', 'T', 'A', 'R', 'T', '\0' },
  { 'H', 'E', 'N', 'D', '\0' },
  { 'L', 'S', 'T', 'A', 'R', 'T', '\0' },
  { 'L', 'E', 'N', 'D', '\0' },
  { 'D', 'H', 'I', 'G', 'H', '\0' },
  { 'D', 'L', 'O', 'W', '\0' },
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

// Sampler state-machine current states
static SampleState state;
static SampleSynchingState synching_state;
static SampleReadyState ready_state;

/**
 * ISR for the Sampler Ticker.
 * */
static void
sampler_tick_routine(void)
{
  sampler_tick.release();
}

static us_timestamp_t t_high = 0;
static us_timestamp_t t_low = 0;

static inline void
synching_state_machine(int value)
{

  switch (synching_state) {
  case S_WAITING:
    if (value == 1) {
      synching_state = S_SYNCHING_HIGH;
      period_timer.reset();
      period_timer.start();
    }
    break;

  case S_SYNCHING_HIGH:
    if (value == 0) {
      period_timer.stop();
      t_high = period_timer.read_high_resolution_us();
      period_timer.reset();
      period_timer.start();
      synching_state = S_SYNCHING_LOW;
    }
    break;

  case S_SYNCHING_LOW:
    if (value == 1) {
      period_timer.stop();
      t_low = period_timer.read_high_resolution_us();

      us_timestamp_t diff = t_high > t_low ? t_high - t_low : t_low - t_high;
      us_timestamp_t shift_high = t_high >> 6;
      us_timestamp_t shift_low = t_low >> 6;

      // printf("(diff, shift_high, shift_low, t_high, t_low) : (%llu, %llu, %llu, %llu, %llu)\r\n", diff, shift_high, shift_low, t_high, t_low);

      if (shift_high >= diff && t_high > 0) {
        state = READY;
        period = (t_high + t_low) >> 2;
        //printf("period: %llu\r\n", period);
        sampler_ticker.detach();
        sampler_ticker.attach_us(sampler_tick_routine, period);

        // This means that we have the value "010" in the MSB of the first byte.
        //printf("Sending signal 4 (0x40)\r\n");
        physic->flags_set(0x04);

        state = READY;
        ready_state = R_INIT;
      } else {
        state = ERROR;
      }
    }
    break;

  default:
    break;
  }
}

static inline void
ready_state_machine(int value)
{
  switch (ready_state) {
  case R_HIGH_START:
    if (value == 0) {
      ready_state = R_LOW_END;
      physic->flags_set(0x02);
    } else {
      ready_state = R_DRIFT_HIGH;
    }
    break;

  case R_LOW_START:
    if (value == 1) {
      ready_state = R_HIGH_END;
      physic->flags_set(0x01);
    } else {
      ready_state = R_DRIFT_LOW;
    }
    break;

  case R_DRIFT_LOW:
    if (value == 1) {
      //printf("There is a one\r\n");
      ready_state = R_HIGH_END;
      physic->flags_set(0x02);
    } else {
      //printf("There is a bad zero\r\n");
      state = ERROR;
    }
    break;

  case R_DRIFT_HIGH:
    if (value == 0) {
      //printf("There is a zero\r\n");
      ready_state = R_LOW_END;
      physic->flags_set(0x02);
    } else {
      //printf("There is a bad one\r\n");
      state = ERROR;
    }
    break;

  case R_HIGH_END:  //passthrough
  case R_LOW_END:
    if (value == 0) {
      ready_state = R_LOW_START;
    } else {
      ready_state = R_HIGH_START;
    }
    break;
  case R_INIT:
    if (value == 0) {
      ready_state = R_LOW_END;
    } else {
      ready_state = R_DRIFT_HIGH;
    }
    break;
  default:
    break;
  }
}

/**
 * Routine called by the sampler thread.
 * */
static void
sampler_th(void)
{
  int value;

  DigitalIn irx(rx);

  while(1) {
    sampler_tick.wait(osWaitForever);
    value = irx;

    //printf("Current state - %s\r\n", SampleStateName[state]);
    switch(state){
      case SYNCHING:
        synching_state_machine(value);
        break;

      case READY:
        //printf("Ready state: %s\r\n", ReadyStateName[ready_state]);
        ready_state_machine(value);
        break;

      case ERROR:
        state = SYNCHING;
        synching_state = S_WAITING;
        ready_state = R_INIT;
        break;

      default:
        break;
    }
  }
}

void
start_sampler_th(PinName _rx, Thread * _phy)
{
  rx = _rx;
  period = MAX_SAMPLING_PERIOD;
  state = SYNCHING;
  synching_state = S_WAITING;
  ready_state = R_INIT;
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