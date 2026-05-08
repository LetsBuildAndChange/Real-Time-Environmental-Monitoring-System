idf_component_register(SRCS "oneshot_read_main.c"
                    PRIV_REQUIRES esp_adc
                    INCLUDE_DIRS "."
                    REQUIRES dht esp_driver_gpio freertos log esp_driver_i2c
                    )
