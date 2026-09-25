#ifndef HTN_WEEK03_DIGITS_H
#define HTN_WEEK03_DIGITS_H

#include <stdint.h>

/* MAX7219 register 1 addresses the rightmost digit, register 8 the leftmost. */
void counter_to_digits(uint32_t value, uint8_t digits[8]);

#endif
