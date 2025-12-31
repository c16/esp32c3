/*
 * ESP32-C3 Bare Metal Program
 * Simple LED blink example using direct register access
 */

#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "bare-metal";

/* GPIO pin for LED - ESP32-C3 typically uses GPIO8 for onboard LED */
#define LED_GPIO    CONFIG_BLINK_GPIO

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 Bare Metal Program Starting...");

    /* Configure GPIO */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "GPIO %d configured as output", LED_GPIO);

    /* Main loop - Toggle LED */
    uint8_t level = 0;
    while (1) {
        ESP_LOGI(TAG, "LED %s", level ? "ON" : "OFF");
        gpio_set_level(LED_GPIO, level);
        level = !level;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
