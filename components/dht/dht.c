#include "dht.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DHT_TIMEOUT_US 1000

static int expect_pulse(gpio_num_t pin, int level)
{
    int count = 0;
    while (gpio_get_level(pin) == level) {
        if (++count > DHT_TIMEOUT_US) {
            return -1;
        }
        esp_rom_delay_us(1);
    }
    return count;
}

esp_err_t dht_read_float_data(
    dht_type_t type,
    gpio_num_t pin,
    float *humidity,
    float *temperature
) {
    uint8_t data[5] = {0};

    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_level(pin, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    if (expect_pulse(pin, 0) < 0) return ESP_FAIL;
    if (expect_pulse(pin, 1) < 0) return ESP_FAIL;

    for (int i = 0; i < 40; i++) {
        if (expect_pulse(pin, 0) < 0) return ESP_FAIL;
        int high_time = expect_pulse(pin, 1);
        if (high_time < 0) return ESP_FAIL;

        data[i / 8] <<= 1;
        if (high_time > 40) {
            data[i / 8] |= 1;
        }
    }

    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) {
        return ESP_FAIL;
    }

    *humidity = data[0];
    *temperature = data[2];

    return ESP_OK;
}