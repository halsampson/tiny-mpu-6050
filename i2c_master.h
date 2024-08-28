// i2c_master.h

#pragma once
#include <avr/io.h>
#include "send.h"

// PORTB pins:
#define SDA _BV(0)
#define SCL _BV(2)

void i2c_init();

extern uint8_t i2c_addr; // set bofer register read/writes

uint8_t i2c_read_reg(uint8_t reg);
int16_t i2c_read_int(uint8_t reg);

uint8_t i2c_write_reg(uint8_t reg, uint8_t data);

const bool I2C_DEBUG = true; // check timing (pullups, ...)