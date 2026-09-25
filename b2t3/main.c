#include <stdint.h>

#include "digits.h"

/* STM32F103C8T6 Blue Pill: clock after reset is HSI = 8 MHz. */
#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_APB2ENR REG32(0x40021018u)
#define GPIOA_CRL   REG32(0x40010800u)
#define GPIOA_BSRR  REG32(0x40010810u)
#define GPIOA_BRR   REG32(0x40010814u)
#define SPI1_CR1    REG32(0x40013000u)
#define SPI1_SR     REG32(0x40013008u)
#define SPI1_DR8    (*(volatile uint8_t *)0x4001300Cu)
#define STK_CTRL    REG32(0xE000E010u)
#define STK_LOAD    REG32(0xE000E014u)
#define STK_VAL     REG32(0xE000E018u)

#define CS_PIN (1u << 4) /* PA4: manual chip select (CS/LOAD). */

static volatile uint32_t milliseconds;

void SysTick_Handler(void)
{
    ++milliseconds;
}

static void wait_ms(uint32_t duration)
{
    const uint32_t start = milliseconds;
    while ((uint32_t)(milliseconds - start) < duration) {
        /* SysTick interrupt updates milliseconds; subtraction tolerates wrap. */
    }
}

static void spi1_init(void)
{
    /* AFIOEN, IOPAEN and SPI1EN on APB2. */
    RCC_APB2ENR |= (1u << 0) | (1u << 2) | (1u << 12);

    /* Drive CS high before changing PA4 into an output. */
    GPIOA_BSRR = CS_PIN;

    /* PA4 GPIO output push-pull 2 MHz: 0x2.
     * PA5 SCK, PA7 MOSI alternate-function push-pull 2 MHz: 0xA.
     * PA6 MISO is unused; leave its reset configuration unchanged. */
    GPIOA_CRL = (GPIOA_CRL & ~((0xFu << 16) | (0xFu << 20) | (0xFu << 28)))
              | (0x2u << 16) | (0xAu << 20) | (0xAu << 28);

    /* SPI mode 0, master, MSB first, 8-bit data, software NSS.
     * BIDIMODE+BIDIOE = one-wire transmit-only: MOSI is used, MISO is not.
     * BR=011: PCLK2 / 16 = 500 kHz with default HSI 8 MHz. */
    SPI1_CR1 = (1u << 15) | (1u << 14) | (1u << 2)
             | (3u << 3) | (1u << 8) | (1u << 9);
    SPI1_CR1 |= (1u << 6); /* SPE: enable SPI1. */
}

static void spi1_send(uint8_t value)
{
    while ((SPI1_SR & (1u << 1)) == 0u) { /* Wait for TXE. */ }
    SPI1_DR8 = value;
}

static void max7219_write(uint8_t address, uint8_t data)
{
    GPIOA_BRR = CS_PIN; /* CS low: start one 16-bit frame. */
    spi1_send(address);
    spi1_send(data);
    while ((SPI1_SR & (1u << 1)) == 0u) { /* TXE after final data write. */ }
    while ((SPI1_SR & (1u << 7)) != 0u) { /* Wait until BUSY clears. */ }
    GPIOA_BSRR = CS_PIN; /* CS high: latch this register/value pair. */
}

static void max7219_show(uint32_t value)
{
    uint8_t digits[8];
    counter_to_digits(value, digits);
    for (uint8_t register_number = 1u; register_number <= 8u; ++register_number) {
        max7219_write(register_number, digits[register_number - 1u]);
    }
}

static void max7219_init(void)
{
    max7219_write(0x0Fu, 0x00u); /* Display test OFF. */
    max7219_write(0x0Cu, 0x00u); /* Shutdown while configuring. */
    max7219_write(0x09u, 0xFFu); /* Code-B decoding for all 8 digits. */
    max7219_write(0x0Bu, 0x07u); /* Scan all 8 digits. */
    max7219_write(0x0Au, 0x03u); /* Low initial brightness, 0..15. */
    max7219_show(0u);
    max7219_write(0x0Cu, 0x01u); /* Normal operation. */
}

int main(void)
{
    /* SysTick = 1 ms at the default 8 MHz core clock. */
    STK_LOAD = 8000u - 1u;
    STK_VAL = 0u;
    STK_CTRL = (1u << 2) | (1u << 1) | (1u << 0);

    spi1_init();
    max7219_init();

    max7219_show(12345678u); /* Check all 8 positions at startup. */
    wait_ms(2000u);

    uint32_t count = 0u;
    for (;;) {
        max7219_show(count);
        wait_ms(1000u);
        count = (count == 99999999u) ? 0u : count + 1u;
    }
}
