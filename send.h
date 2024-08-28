#pragma once
#include <avr/io.h>
#include <stdlib.h>

#define F_CPU 16000000

// PORTB pins:
#define TxDbit  3       // any but 5 (which requires RSTDISBL & HV to restore)
#define TxD     _BV(TxDbit)  // Violet

#define BAUD_RATE 921600

const bool txdInvert = false;

void send(char ch);
void send(const char* s);

void sendHex(uint32_t i);
void send(int32_t i);
void send(int i);
void send(uint16_t i);

