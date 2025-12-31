/*
 * ESP32-C3 TRUE Bare Metal Program
 * Direct register access without FreeRTOS
 * Includes flash memory testing
 */

#include <stdint.h>
#include <string.h>
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "hal/gpio_ll.h"
#include "esp_attr.h"
#include "esp_rom_sys.h"
#include "esp_partition.h"
#include "esp_flash.h"
#include "esp_log.h"

/* GPIO pin for LED - ESP32-C3 typically uses GPIO8 for onboard LED */
#define LED_GPIO    8

/* Flash test configuration */
#define FLASH_TEST_ENABLED      1
#define FLASH_TEST_BLOCK_SIZE   4096    /* 4KB blocks */
#define FLASH_TEST_BUFFER_SIZE  256     /* Test buffer size */

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

/* LED blink patterns for status indication */
static void blink_pattern_success(void)
{
    /* Fast blinks = success */
    for (int i = 0; i < 5; i++) {
        gpio_set_high(LED_GPIO);
        delay_ms(100);
        gpio_set_low(LED_GPIO);
        delay_ms(100);
    }
}

static void blink_pattern_error(void)
{
    /* Slow long blinks = error */
    for (int i = 0; i < 3; i++) {
        gpio_set_high(LED_GPIO);
        delay_ms(500);
        gpio_set_low(LED_GPIO);
        delay_ms(500);
    }
}

static void blink_pattern_testing(void)
{
    /* Quick double blink = testing in progress */
    gpio_set_high(LED_GPIO);
    delay_ms(50);
    gpio_set_low(LED_GPIO);
    delay_ms(50);
    gpio_set_high(LED_GPIO);
    delay_ms(50);
    gpio_set_low(LED_GPIO);
    delay_ms(200);
}

#if FLASH_TEST_ENABLED

static const char *TAG = "flash-test";

/* Flash test result structure */
typedef struct {
    uint32_t total_size;
    uint32_t tested_size;
    uint32_t errors;
    uint32_t blocks_tested;
} flash_test_result_t;

/* Test flash with pattern */
static esp_err_t test_flash_block(uint32_t address, uint32_t size)
{
    uint8_t write_buf[FLASH_TEST_BUFFER_SIZE];
    uint8_t read_buf[FLASH_TEST_BUFFER_SIZE];
    esp_err_t err;

    /* Generate test pattern (alternating 0xAA and 0x55) */
    for (int i = 0; i < FLASH_TEST_BUFFER_SIZE; i++) {
        write_buf[i] = (i & 1) ? 0xAA : 0x55;
    }

    /* Erase the sector first */
    err = esp_flash_erase_region(NULL, address, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Flash erase failed at 0x%lx: %s", address, esp_err_to_name(err));
        return err;
    }

    /* Write test pattern */
    err = esp_flash_write(NULL, write_buf, address, FLASH_TEST_BUFFER_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Flash write failed at 0x%lx: %s", address, esp_err_to_name(err));
        return err;
    }

    /* Read back */
    err = esp_flash_read(NULL, read_buf, address, FLASH_TEST_BUFFER_SIZE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Flash read failed at 0x%lx: %s", address, esp_err_to_name(err));
        return err;
    }

    /* Verify */
    if (memcmp(write_buf, read_buf, FLASH_TEST_BUFFER_SIZE) != 0) {
        ESP_LOGE(TAG, "Flash verify failed at 0x%lx", address);
        return ESP_FAIL;
    }

    return ESP_OK;
}

/* Test all available flash outside program area */
static void test_flash_memory(flash_test_result_t *result)
{
    esp_err_t err;
    uint32_t flash_size = 0;

    memset(result, 0, sizeof(flash_test_result_t));

    /* Get flash chip size */
    err = esp_flash_get_size(NULL, &flash_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get flash size: %s", esp_err_to_name(err));
        result->errors++;
        return;
    }

    result->total_size = flash_size;
    ESP_LOGI(TAG, "Flash size: %lu bytes (%.2f MB)", flash_size, flash_size / (1024.0 * 1024.0));

    /* Find the storage partition to test */
    const esp_partition_t *storage_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_ANY,
        NULL
    );

    if (storage_partition == NULL) {
        ESP_LOGW(TAG, "No storage partition found, will test free space");

        /* Test upper half of flash (assuming program is in lower half) */
        uint32_t test_start = flash_size / 2;
        uint32_t test_end = flash_size;

        ESP_LOGI(TAG, "Testing flash range: 0x%lx to 0x%lx", test_start, test_end);

        for (uint32_t addr = test_start; addr < test_end; addr += FLASH_TEST_BLOCK_SIZE) {
            blink_pattern_testing();

            err = test_flash_block(addr, FLASH_TEST_BLOCK_SIZE);
            if (err == ESP_OK) {
                result->tested_size += FLASH_TEST_BUFFER_SIZE;
                result->blocks_tested++;
            } else {
                result->errors++;
                ESP_LOGE(TAG, "Block test failed at 0x%lx", addr);
            }

            /* Test a limited number of blocks to save time */
            if (result->blocks_tested >= 100) {
                ESP_LOGI(TAG, "Tested 100 blocks, stopping test");
                break;
            }
        }
    } else {
        /* Test the storage partition */
        ESP_LOGI(TAG, "Found storage partition at 0x%lx, size: %lu bytes",
                 storage_partition->address, storage_partition->size);

        uint32_t test_end = storage_partition->address + storage_partition->size;

        for (uint32_t addr = storage_partition->address;
             addr < test_end;
             addr += FLASH_TEST_BLOCK_SIZE) {

            blink_pattern_testing();

            err = test_flash_block(addr, FLASH_TEST_BLOCK_SIZE);
            if (err == ESP_OK) {
                result->tested_size += FLASH_TEST_BUFFER_SIZE;
                result->blocks_tested++;
            } else {
                result->errors++;
                ESP_LOGE(TAG, "Block test failed at 0x%lx", addr);
            }

            /* Test a limited number of blocks */
            if (result->blocks_tested >= 100) {
                ESP_LOGI(TAG, "Tested 100 blocks, stopping test");
                break;
            }
        }
    }

    /* Print results */
    ESP_LOGI(TAG, "=== Flash Test Results ===");
    ESP_LOGI(TAG, "Total flash size: %lu bytes", result->total_size);
    ESP_LOGI(TAG, "Tested data: %lu bytes", result->tested_size);
    ESP_LOGI(TAG, "Blocks tested: %lu", result->blocks_tested);
    ESP_LOGI(TAG, "Errors: %lu", result->errors);

    if (result->errors == 0) {
        ESP_LOGI(TAG, "Flash test PASSED!");
        blink_pattern_success();
    } else {
        ESP_LOGE(TAG, "Flash test FAILED with %lu errors!", result->errors);
        blink_pattern_error();
    }
}

#endif /* FLASH_TEST_ENABLED */

void app_main(void)
{
    /* Initialize LED GPIO as output */
    gpio_init_output(LED_GPIO);

    /* Initial state - LED OFF */
    gpio_set_low(LED_GPIO);

#if FLASH_TEST_ENABLED
    /* Run flash memory test on startup */
    flash_test_result_t test_result;

    ESP_LOGI(TAG, "Starting flash memory test...");
    ESP_LOGI(TAG, "This will test flash memory outside program area");

    /* Brief delay to allow serial output */
    delay_ms(100);

    test_flash_memory(&test_result);

    /* Pause after test to see results */
    delay_ms(2000);
#endif

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
