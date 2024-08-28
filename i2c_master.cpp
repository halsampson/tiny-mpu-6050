#include "i2c_master.h"

#define USICR_MODE (_BV(USIWM1) | _BV(USICS1) | _BV(USICLK)) // software clock strobe USITC

void i2c_init() {
  DDRB  |= SCL | SDA;  // Master driven
  PORTB |= SCL | SDA;  // 20-50K ohm ATtiny pullups  (BY-521 has 2.2K ohm pullups)

  USIDR = 0xFF;
  USICR = USICR_MODE;
  USISR = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC); // clear flags and counter
}


// I2C Fast mode 400 kHz specs:
void i2c_setup_time() {
  __builtin_avr_delay_cycles(0.6 * F_CPU / 1000000 - 7); // 0.6 us
}

void i2c_hold_time() { // e.g. SCL Lo
  __builtin_avr_delay_cycles(1.3 * F_CPU / 1000000 - 7); // 1.3 us
}


uint8_t i2c_transfer(bool ack = false) {
  USISR = _BV(USISIF) | _BV(USIOIF) | _BV(USIPF) | _BV(USIDC) | (ack ? 16 - 1 : 16 - 15);  // edges to overflow for 1 or 8 bits
  do {  // transfer
    i2c_hold_time();  // SCL Lo
    USICR = USICR_MODE | _BV(USITC); // SCL / Hi
    if (I2C_DEBUG) while (!(PINB & SCL)); // Wait for SCL to go Hi
    i2c_setup_time();
    USICR = USICR_MODE | _BV(USITC); // SCL \ Lo
  } while (!(USISR & _BV(USIOIF))); // until counter overflow
  i2c_hold_time();

  USIDR = 0xFF; // SDA Hi = MSB of USI Data Register
  DDRB |= SDA;

  return USIBR;
}

bool i2c_write(uint8_t data) {
  USIDR = data;
  i2c_transfer();

  DDRB &= ~SDA; // read NACK bit
  uint8_t nack = i2c_transfer(true);  // Lo = ACK
  if (I2C_DEBUG && (nack & 1)) send("wr nack! ");

  return nack & 1;
}

void i2c_start(void) {
  PORTB |= SCL;  // SCL Hi
  PORTB &= ~SDA; // SDA Lo
  i2c_setup_time();

  PORTB &= ~SCL; // SCL Lo
  i2c_hold_time();

  PORTB |= SDA;  // SDA Hi
}

uint8_t i2c_addr; // set before calls below: typically changes infrequently

void i2c_start(uint8_t reg) {
  i2c_start();
  i2c_write(i2c_addr << 1);
  i2c_write(reg);
}

uint8_t i2c_read(uint8_t NACK = 0xFF) { // 0: more data coming;   MSB set: last byte
  DDRB &= ~SDA; // receive input
  uint8_t data = i2c_transfer();

  USIDR = NACK;
  i2c_transfer(true);

  return data;
}

void i2c_stop() { // Stop: SDA Lo / Hi while SCL Hi
  PORTB &= ~SDA;  // SDA Lo
  PORTB |= SCL;   // SCL Hi
  if (I2C_DEBUG) while (!(PINB & SCL));
  i2c_setup_time();

  PORTB |= SDA;   // SDA Hi
  i2c_hold_time();

  if (I2C_DEBUG && !(USISR & _BV(USIPF))) send("stop! ");
}

void i2c_start_read(uint8_t reg) {
  i2c_start(reg);
  i2c_start(); // restart to read
  i2c_write(i2c_addr << 1 | 1); // direction = in
}

uint8_t i2c_read_reg(uint8_t reg) {
  i2c_start_read(reg);
  uint8_t data = i2c_read();
  i2c_stop();
  return data;
}

int16_t i2c_read_int(uint8_t reg) {
  i2c_start_read(reg);
  int16_t i = (int16_t)i2c_read(0) << 8 | i2c_read();
    // or more bytes with i2c_read(0)
  i2c_stop();
  return i;
}

uint8_t i2c_write_reg(uint8_t reg, uint8_t data) {
  i2c_start(reg);
  uint8_t nack = i2c_write(data);
  i2c_stop();
  return nack;
}
