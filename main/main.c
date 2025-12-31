/*
 * ESP32-C3 TRUE Bare Metal Program
 * Direct register access without FreeRTOS
 */

#include <stdint.h>
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "hal/gpio_ll.h"
#include "esp_attr.h"
#include "esp_rom_sys.h"

/* GPIO pin for LED - ESP32-C3 typically uses GPIO8 for onboard LED */
#define LED_GPIO    8

/* GPIO Register Addresses for ESP32-C3 */
#define GPIO_ENABLE_REG         (0x60004020)
#define GPIO_OUT_REG            (0x60004004)
#define GPIO_FUNC_OUT_SEL_CFG   (0x60004554)

/* Simple bare metal delay function (approximate) */
static inline void IRAM_ATTR delay_cycles(uint32_t cycles)
{
    uint32_t start = esp_rom_get_ccount();
    uint32_t end = start + cycles;

    while ((esp_rom_get_ccount() - start) < cycles) {
        __asm__ __volatile__("nop");
    }
}

/* Delay in milliseconds (assuming 160MHz CPU clock) */
static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_cycles(160000);  /* 160MHz = 160,000 cycles per ms */
    }
}

/* Direct register write */
static inline void write_reg(uint32_t addr, uint32_t val)
{
    *((volatile uint32_t *)addr) = val;
}

/* Direct register read */
static inline uint32_t read_reg(uint32_t addr)
{
    return *((volatile uint32_t *)addr);
}

/* Set GPIO bit */
static inline void gpio_set_high(uint8_t gpio_num)
{
    uint32_t val = read_reg(GPIO_OUT_REG);
    write_reg(GPIO_OUT_REG, val | (1 << gpio_num));
}

/* Clear GPIO bit */
static inline void gpio_set_low(uint8_t gpio_num)
{
    uint32_t val = read_reg(GPIO_OUT_REG);
    write_reg(GPIO_OUT_REG, val & ~(1 << gpio_num));
}

/* Initialize GPIO as output */
static void gpio_init_output(uint8_t gpio_num)
{
    /* Enable GPIO output */
    uint32_t enable_val = read_reg(GPIO_ENABLE_REG);
    write_reg(GPIO_ENABLE_REG, enable_val | (1 << gpio_num));

    /* Configure pin function as GPIO */
    uint32_t func_addr = GPIO_FUNC_OUT_SEL_CFG + (gpio_num * 4);
    write_reg(func_addr, 0x80);  /* Simple output mode */
}

void app_main(void)
{
    /* Initialize LED GPIO as output */
    gpio_init_output(LED_GPIO);

    /* Initial state - LED OFF */
    gpio_set_low(LED_GPIO);

    /* Main loop - Toggle LED every second */
    while (1) {
        /* LED ON */
        gpio_set_high(LED_GPIO);
        delay_ms(1000);

        /* LED OFF */
        gpio_set_low(LED_GPIO);
        delay_ms(1000);
    }
}
