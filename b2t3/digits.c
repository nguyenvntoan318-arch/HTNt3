#include "digits.h"

void counter_to_digits(uint32_t value, uint8_t digits[8])
{
    value %= 100000000u;
    for (uint8_t i = 0u; i < 8u; ++i) {
        digits[i] = (uint8_t)(value % 10u);
        value /= 10u;
    }
}
