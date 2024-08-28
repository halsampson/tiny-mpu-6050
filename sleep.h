#pragma once
#include <avr/io.h>
#include "send.h"

#if 0 // inline delayCycles for intervals below 14 cycles
  #define DELAY_INLINE inline
  #define CALL_RET_CYCLES 0
#else
  #define DELAY_INLINE
  #define CALL_RET_CYCLES (3 + 4) // RCALL RET
#endif
// Beware: parameter passing cycles vary:
//  MOVW (1 cycle) or LD H; LD L (2 cycles)

DELAY_INLINE void delayCycles(int cycles);
  void testDelayCycles();

void usleep(uint16_t us);
void msleep(uint16_t ms);
void sleep(uint16_t secs);

void WDT_off();



