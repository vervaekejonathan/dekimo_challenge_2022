#pragma once


// https://github.com/arduino/ArduinoCore-megaavr
// https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega4808-09-DataSheet-DS40002173C.pdf

static volatile TCB_t* _timer =
#if defined(MILLIS_USE_TIMERB0)
&TCB0;
#elif defined(MILLIS_USE_TIMERB1)
&TCB1;
#elif defined(MILLIS_USE_TIMERB2)
&TCB2;
#elif defined(MILLIS_USE_TIMERB3)
&TCB3;
#else
// fallback to TCB0 (every platform has it)
&TCB0;
#endif

// #define EveryTimerB_CLOCMODE TCB_CLKSEL_CLKTCA_gc  // 250 kHz ~ 4 us
// #define EveryTimerB_CLOCMODE TCB_CLKSEL_CLKDIV2_gc //   8 MHz ~ 0.125 us
// #define EveryTimerB_CLOCMODE TCB_CLKSEL_CLKDIV_gc  //  16 MHz ~ 0.0625 us  

void init_nanos() {
  _timer->CTRLA = 0x02; //  16 MHz ~ 0.0625 us  
}

inline unsigned long nanos_start() {
  _timer->CCMP = 0;
  _timer->CTRLA |= TCB_ENABLE_bm;
  _timer->CNT = 0;
}

inline uint16_t nanos_end() {
  uint8_t ticks = _timer->CNTL;
  _timer->CTRLA &= ~TCB_ENABLE_bm;
  _timer->INTFLAGS = TCB_CAPT_bm;  // writing to the INTFLAGS register will clear the interrupt request flag
  return (0.625 * ticks * 1000);
}
