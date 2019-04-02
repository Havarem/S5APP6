#include "sampler_man.h"

Semaphore sampler_tick;

/**
 * Sampler state
 * */
typedef enum {
  WAITING = 0,
  SYNCHING_HIGH = 1,
  SYNCHING_LOW = 2,
  INIT = 3,
  VAR = 4,
  EXP_1 = 5,
  ERROR = 6
} SampleState;

const static char StateName[7][9] = {
  {'W', 'A', 'I', 'T', 'I', 'N', 'G', '\0'},
  {'S', 'Y', 'N', 'C', '-', 'H', '\0'},
  {'S', 'Y', 'N', 'C', '-', 'L', '\0'},
  {'I', 'N', 'I', 'T', '\0'},
  {'V', 'A', 'R', '\0'},
  {'E', 'X', 'P', '-', '1', '\0'},
  { 'E', 'R', 'R', 'O', 'R', '\0' }
};

// Sampler Thread instance
static Thread sampler(osPriorityRealtime, 2048);
// Physic-layer thread reference
static Thread *physic;

static InterruptIn signal(p21);

static Timer period_timer;
// Sampler period in nano-second
static us_timestamp_t double_period;
// Sampler period in nano-second
static us_timestamp_t period;

// Sampler state-machine current states
static SampleState state;

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
static us_timestamp_t interval;

static inline void
state_machine(int value)
{
  us_timestamp_t interval_range;
  us_timestamp_t interval_margin;
  uint32_t multiple = 0;
  //Logger::getLogger().logInfo(DEBUG_SAMPLER, "Flag: %s", StateName[state]);

  if (state == INIT || state == VAR || state == EXP_1) {
    period_timer.stop();
    interval = period_timer.read_high_resolution_us();
    period_timer.reset();
    period_timer.start();

    interval_range = interval >> 6;
    interval_margin = interval - interval_range;

    if (interval_margin > double_period) {
      state = ERROR;
    } else if (interval_margin > period) {
      multiple = 2;
      if (state == EXP_1) {
        state = ERROR;
      }
    } else {
      if (state == VAR) {
        state = EXP_1;
      } else if (state == EXP_1) {
        state = VAR;
      }
      multiple = 1;
    }
  }

  switch (state) {
  case WAITING:
    if (value == 1) {
      state = SYNCHING_HIGH;
      period_timer.reset();
      period_timer.start();
    }
    break;

  case SYNCHING_HIGH:
    if (value == 0) {
      period_timer.stop();
      t_high = period_timer.read_high_resolution_us();
      period_timer.reset();
      period_timer.start();
      state = SYNCHING_LOW;
    }
    break;

  case SYNCHING_LOW:
    if (value == 1) {
      period_timer.stop();
      t_low = period_timer.read_high_resolution_us();
      period_timer.reset();
      period_timer.start();

      us_timestamp_t diff = t_high > t_low ? t_high - t_low : t_low - t_high;
      us_timestamp_t shift_high = t_high >> 6;

      if (shift_high >= diff && t_high > 0) {
        double_period = t_high;
        period = t_high >> 1;

        // This means that we have the value "010" in the MSB of the first byte.
        physic->flags_set(0x04);

        state = INIT;
      } else {
        //Logger::getLogger().logDebug(DEBUG_SAMPLER, "Weird rise not equal to 1");
        state = ERROR;
      }
    }
    break;

  case INIT:
    if (multiple == 2) {
      physic->flags_set((value == 0) ? 0x02 : 0x01);
      state = VAR;
    } else {
      //::getLogger().logDebug(DEBUG_SAMPLER, "No 2 periods in INIT");
      state = ERROR;
    }
    break;

  case VAR:
    //Logger::getLogger().logDebug(DEBUG_SAMPLER, "VAR");
    physic->flags_set((value == 0) ? 0x02 : 0x01);
    break;

  case EXP_1:
    //Logger::getLogger().logDebug(DEBUG_SAMPLER, "EXP_1");
    break;

  case ERROR:
    //Logger::getLogger().logDebug(DEBUG_SAMPLER, "We have an error");
    if (value == 0) {
      state = WAITING;
    }
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

  while(1) {
    sampler_tick.wait(osWaitForever);
    value = signal;
    //Logger::getLogger().logDebug(DEBUG_SAMPLER, "Valeur lu: %d", value);
    state_machine(value);
  }
}

void
start_sampler_th(Thread * _phy)
{
  state = WAITING;
  physic = _phy;
  sampler.start(sampler_th);
  printf("Sampler id: %x\r\n", sampler.get_id());

  signal.rise(sampler_tick_routine);
  signal.fall(sampler_tick_routine);
}

us_timestamp_t
get_current_period(void)
{
  return period;
}