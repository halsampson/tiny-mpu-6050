// MPU-6050 temperature reading

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

#include "send.h"
#include "sleep.h"
#include "i2c_master.h"

#define MPU6050  0x68 // I2C address | AD0

void freqOut() { // Timer 0 Fast PWM ~1 MHz out to pin FREQO
  #define FREQO   _BV(0)  // OC0A   Blue
  DDRB |= FREQO;
  OCR0A = F_CPU / 1000000 / 2 - 1; // CK divisor
  // OCR0B = (OCR0A + 1) / 2; // 50% duty cycle -- not used if toggling
  TCCR0A = _BV(COM0A0) | _BV(WGM01) | _BV(WGM00);  // toggle
  TCCR0B = _BV(WGM02) | _BV(CS00);
  // https://www.ee-diary.com/2021/08/programming-arduino-timer-0-in-fast-pwm.html
}


int main(void) {
  OSCCAL -= 4; // adjust calibration

  send("\nMPU-6060 degC " __TIME__ "\n");

#if 0
  freqOut();
  sleep(5); // to check 1 MHz accuracy
#endif

  i2c_init();
  i2c_addr = MPU6050;

#if 0
  i2c_write_reg(25, 0xFF);   // sample rate divider / 256
  i2c_write_reg(26, 6);      // CONFIG 1 kHz
  i2c_write_reg(35, 0x80);   // degC -> FIFO EN
  i2c_write_reg(106, 0x80);  // FIFO_EN
#endif
  i2c_write_reg(107, 0x00);  // Cycle, 8 MHz, enable temperature sensor
  i2c_write_reg(108, 0x3F);  // all motion standby for lowest self-heating (remove green power LED also)

 while (1) {
   int16_t degC100ths = (int32_t)i2c_read_int(65) * 5 / 17 + 3653;  //  * 100 / 340 reduced to 5 / 17
   send(degC100ths); send('\n');
   sleep(1);  // TODO: ~4Hz updates could be averaged
 }
}

