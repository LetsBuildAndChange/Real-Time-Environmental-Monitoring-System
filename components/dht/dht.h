#pragma once

#include "esp_err.h"
#include "driver/gpio.h"

typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22
} dht_type_t;

esp_err_t dht_read_float_data(
    dht_type_t type,
    gpio_num_t gpio_num,
    float *humidity,
    float *temperature
);