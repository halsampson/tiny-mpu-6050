#include "send.h"

inline void txStop() {  // init only
  if (txdInvert)
  PORTB &= ~TxD; // stop bit
  else
  PORTB |= TxD; // stop bit
  DDRB |= TxD;
}

void send(char ch) {
  const int BAUD_DIV = ((F_CPU + BAUD_RATE / 2)/ BAUD_RATE);
  uint16_t out = (uint16_t)ch << 2 | 0x401; // stop  data  start  prev stop
  out = out ^ (out >> 1); // calculate toggles from previous bits
  out <<= TxDbit; // align with TxD pin

  char bits = 1 + 8 + 1;
  do {
    PINB = out & TxD; // toggle if (out & TxD)
    out >>= 1;
    __builtin_avr_delay_cycles(BAUD_DIV - 8);  // check
  } while (--bits);
}

void send(const char* s) {
  txStop();  // only needed once
  static uint8_t col;
  while (*s) {
    if (*s < '0') // punctuation/...
    col = 0;
    else ++col;
    send(*s++);
  }
  if (col) send(' '); // no separator at end: provide space
}

void sendHex(uint32_t i) {
  char buf[9];
  send(ltoa(i, buf, 16));
}

void send(int32_t i) {
  char buf[9];
  send(ltoa(i, buf, 10));
}

void send(int i) {
  char buf[7];
  send(itoa(i, buf, 10));
}

void send(uint16_t i) {
  char buf[6];
  send(itoa(i, buf, 10));
}
