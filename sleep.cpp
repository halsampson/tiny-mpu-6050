#include "sleep.h"
#include <avr/interrupt.h>

#if 0 // inline delayCycles for intervals below 14 cycles
  #define DELAY_INLINE inline
  #define CALL_RET_CYCLES 0
#else
  #define DELAY_INLINE
  #define CALL_RET_CYCLES (3 + 4) // RCALL RET
#endif

// Beware: parameter passing cycles vary:
//  MOVW (1 cycle) or LD H; LD L (2 cycles)

DELAY_INLINE void delayCycles(int cycles) {         //(3 RCALL)
  cycles -= 1 + 3 + 3  + CALL_RET_CYCLES;           // 1
  while ((cycles -= 4) >= 0);                       // 3 min

  // delay by remainder -4 -3 -2 -1 = FC FD FE FF      3..7 cycles
  asm volatile("CPI %[d],-3" : : [d] "r" (cycles)); // 1
  asm volatile("BRLO .+6");                         // 1 / 2
  asm volatile("BREQ .+4");                         // 1 / 2
  asm volatile("SBRC %[d],0" : : [d] "r" (cycles)); // 1 / 2
  asm volatile("RJMP .+1");                         // 2
  asm volatile("NOP");                              // 1
}                                                   //(4 RET)

// https://gcc.gnu.org/onlinedocs/gcc/extensions-to-the-c-language-family/how-to-use-inline-assembly-language-in-c-code.html
// https://gcc.gnu.org/onlinedocs/gcc-4.3.1/gcc/Extended-Asm.html#Extended-Asm

void testDelayCycles() {
  TCCR1 = 1;
  for (int d = 0; d < 50; ++d) {
    send(d);
    TCNT1 = 0;
    delayCycles(d); // LD not timed
    send(TCNT1);
    send('\n');
  }
}

void usleep(uint16_t us) {
  while (us--) __builtin_avr_delay_cycles(F_CPU / 1000000 - 3);
}

void msleep(uint16_t ms) {
  if (ms <= 65) {
    usleep(ms << 10);  // more accurate than WDT
    return;
  }
  ms >>= 4;
  asm("WDR");
  do {
    WDTCR = _BV(WDIE) | _BV(WDCE) | _BV(WDIF);  // ~16 ms
    asm("SLEEP");
  } while (--ms);
}


void WDT_off() {
  asm("WDR");
  MCUSR = 0; // clear WDRF
  WDTCR = _BV(WDCE) | _BV(WDE);
  WDTCR = 0;
}

ISR(WDT_vect) {
  WDT_off();
}

void sleep(uint16_t secs) {
  MCUCR = _BV(SE);  // SleepEnable, Idle mode
  sei();

  uint8_t wdp = 9;
  uint8_t wdt_secs = 8;
  asm("WDR");
  while (1) {
    while (secs >= wdt_secs) {
      WDTCR = _BV(WDIF) | _BV(WDIE) | (wdp & 8) << (WDP3 - 3) | _BV(WDCE) | (wdp & 7);
      asm("SLEEP");
      if (!(secs -= wdt_secs)) return;
    }
    wdt_secs >>= 1;
    --wdp;
  }
}
