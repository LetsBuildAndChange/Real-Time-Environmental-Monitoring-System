/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"
#include "dht.h"
#include <HD44780.h>
#include "driver/i2c_master.h"
const static char *TAG = "EXAMPLE";

// LCD configuration
#define LCD_ADDR 0x27 // common address, but may be 0x3F
#define LCD_COLS 16
#define LCD_ROWS 2
#define GPIO_LCD_SDA 18
#define GPIO_LCD_SCL 19
#define GPIO_LDR_LED 2
// DHT11 configuration
#define GPIO_DHT_DATA 25
#define DHT_TYPE DHT_TYPE_DHT11
#define GPIO_HUMID_LED 0
#define GPIO_TEMP_LED 14
// ADC configuration
#define GPIO_ADC_INPUT 34

float lastFiveTemps[5] = {0}; 
float lastFiveHumids[5] = {0};
float runningTempSum = 0;
float runningHumidSum = 0;
static int raw_input;
static int voltage;

    void gpio_init(void)
{

    gpio_reset_pin(GPIO_DHT_DATA);
    gpio_reset_pin(GPIO_ADC_INPUT);
    gpio_reset_pin(GPIO_LDR_LED);
    gpio_reset_pin(GPIO_HUMID_LED);
    gpio_reset_pin(GPIO_TEMP_LED);
    gpio_set_direction(GPIO_ADC_INPUT, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_DHT_DATA, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_LDR_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_HUMID_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_TEMP_LED, GPIO_MODE_OUTPUT);


}

void computeFirstTempAverage(void){
    float sum = 0;
    for(int j = 0; j < 5; j++){
        sum += lastFiveTemps[j];
    }
    printf("\n");
    float average = sum / 5.0;
    printf("Average Temperature: %.1f C\n", average);     
    runningTempSum = sum;
}

void computeFirstHumidAverage(void){
    float sum = 0;
    for(int j = 0; j < 5; j++){
        sum += lastFiveHumids[j];
    }
    printf("\n");
    float average = sum / 5.0;
    printf("Filtered Humidity: %.1f %%\n", average);     
    runningHumidSum = sum;
}

void print_temp_array(void){
    for(int j = 0; j < 5; j++){
        printf("%.1f F ", lastFiveTemps[j]);
    }
    printf("\n");
}

void print_humid_array(void){
    for(int j = 0; j < 5; j++){
        printf("%.1f %% ", lastFiveHumids[j]);
    }
    printf("\n");
}

void app_main(void)
{   
    // Initialize GPIOs
    gpio_init();
    // Initialize the LCD
    LCD_init(LCD_ADDR, GPIO_LCD_SDA, GPIO_LCD_SCL, LCD_COLS, LCD_ROWS);

    LCD_clearScreen();
    LCD_setCursor(0, 0);
    LCD_writeStr("ESP32 LCD OK");

    LCD_setCursor(0, 1);
    LCD_writeStr("Temp/Hum next");
    // Initialize the ADC unit and get a handle to it
    adc_oneshot_unit_handle_t adc_handle; // reads the adc unit data and stores it in adc_handle
    adc_oneshot_unit_init_cfg_t init_config = { // ADC unit 
        .unit_id = ADC_UNIT_1, // Use ADC unit 1 bc thats connected to pin 34
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    // configures the adc channel and its parameters
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,  
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    // three parameters: adc_handle: handle of the adc unit, ADC_CHANNEL_6: channel to be configured &config: pointer to the configuration structure
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_6, &config));
    int i = 0;
    int k = 0;
    float runningTempSum = 0;

        while(1){

        int lightlowthreshold = 1000;
        int lightupperthreshold = 3700;
        float temperature = 0.0;
        float humidity = 0.0;
        float averageTemp = 0.0;
        float averageHumid = 0.0;
        
        if (dht_read_float_data(DHT_TYPE, GPIO_DHT_DATA, &humidity, &temperature) == ESP_OK) {
            // Reads LDR(adc) data from the specified channel and stores it in raw_input
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_6, &raw_input));
            // Control the LDR LED based on the raw ADC value
            if(raw_input < lightlowthreshold){
                ESP_LOGW(TAG, "it is dark RawData:%d", raw_input);
                gpio_set_level(GPIO_LDR_LED, 1);
            }
            else if(raw_input > lightupperthreshold){
                ESP_LOGI(TAG, "it is bright RawData:%d", raw_input);
                gpio_set_level(GPIO_LDR_LED, 1);
            }
            else{
                gpio_set_level(GPIO_LDR_LED, 0);
            }
            voltage = (raw_input*3300) / 4095;
            // Log the raw ADC value and the corresponding voltage
            ESP_LOGI(TAG, "ADC%d Channel%d Data:%d Voltage:%d mV", ADC_UNIT_1, ADC_CHANNEL_6, raw_input, voltage);
            printf("DHT11 READ: \n");
            temperature = (temperature * 9/5) + 32; // Convert to Fahrenheit
            // Compute the average for last five Temperature readings
            if(i < 5){
                // not full yet, add to array
                printf("Raw Temperature: %.1f F\n", temperature);
                lastFiveTemps[i] = temperature;
                float sum = 0;
                for(int j = 0; j <= i; j++){
                    sum += lastFiveTemps[j];
                }
                averageTemp = sum / (i + 1);
                printf("Filtered Temperature: %.1f F\n", averageTemp);
                i++;
                print_temp_array();
                runningTempSum = sum;
                // first time full, print values and average
                // if(i == 5){
                //     // print_temp_array();
                //     computeFirstTempAverage();
                // }
            }
            else{
                // this is when its full, shift values and add new one
                // update running sum by removing the first value and adding the new temperature
                printf("Raw Temperature: %.1f F\n", temperature);
                runningTempSum = (runningTempSum - lastFiveTemps[0]) + temperature;
                for(int j = 1; j < 5; j++){
                    lastFiveTemps[j - 1] = lastFiveTemps[j];
                }
                lastFiveTemps[4] = temperature;
                print_temp_array();
                averageTemp =  runningTempSum / 5.0;
                printf("Filtered Temperature: %.1f F\n", averageTemp);
            }

            // Compute the average for last five Humidity readings
            if(k < 5){
                printf("Raw Humidity: %.1f %%\n", humidity);
                lastFiveHumids[k] = humidity;
                int sum = 0;
                for(int j = 0; j <= k; j++){
                    sum += lastFiveHumids[j];
                }
                averageHumid = sum / (k + 1.0);
                printf("Filtered Humidity: %.1f %%\n", averageHumid);
                k++;
                print_humid_array();
                runningHumidSum = sum;
                // if(k == 5){
                //     computeFirstHumidAverage();
                // }
            }
            else{
                // this is when its full, shift values and add new one
                // update running sum by removing the first value and adding the new humidity
                printf("Raw Humidity: %.1f %%\n", humidity);
                runningHumidSum = (runningHumidSum - lastFiveHumids[0]) + humidity;
                for(int j = 1; j < 5; j++){
                    lastFiveHumids[j - 1] = lastFiveHumids[j];
                }
                lastFiveHumids[4] = humidity;
                print_humid_array();
                averageHumid = runningHumidSum / 5.0;
                printf("Filtered Humidity: %.1f %%\n", averageHumid);
            }


        }
         else {
            printf("Failed to read\n");
        }

        // Control the temperature LED based on the filtered temperature value
        if(averageTemp < 60.0){
            ESP_LOGW(TAG, "Temperature is low: %.1f F", averageTemp);
            gpio_set_level(GPIO_TEMP_LED, 1);
        }
        else if(averageTemp > 75.0){
            ESP_LOGI(TAG, "Temperature is high: %.1f F", averageTemp);
            gpio_set_level(GPIO_TEMP_LED, 1);
        }
        else{
            gpio_set_level(GPIO_TEMP_LED, 0);
        }

        // Control the humidity LED based on the filtered humidity value
        if(averageHumid < 20.0){
            ESP_LOGW(TAG, "Humidity is low: %.1f %%", averageHumid);
            gpio_set_level(GPIO_HUMID_LED, 1);
        }
        else if(averageHumid > 85.0){
            ESP_LOGI(TAG, "Humidity is high: %.1f %%", averageHumid);
            gpio_set_level(GPIO_HUMID_LED, 1);
        }
        else{
            gpio_set_level(GPIO_HUMID_LED, 0);
        }
        
        // Configure the LCD display with the current temperature, humidity, and raw ADC value
        char line1[9];
        char line2[9];
        char secHalfline1[7];

        snprintf(line1, sizeof(line1), "T: %.1fF", averageTemp);
        snprintf(line2, sizeof(line2), "H: %.1f%%", averageHumid);
        snprintf(secHalfline1, sizeof(secHalfline1), "L:%d", raw_input);

        LCD_setCursor(0, 0);
        LCD_writeStr("                ");
        LCD_setCursor(0, 0);
        LCD_writeStr(line1);

        LCD_setCursor(9, 0);
        LCD_writeStr("                ");
        LCD_setCursor(9, 0);
        LCD_writeStr(secHalfline1);


        LCD_setCursor(0, 1);
        LCD_writeStr("                ");
        LCD_setCursor(0, 1);
        LCD_writeStr(line2);
        vTaskDelay(2500/ portTICK_PERIOD_MS);
    }
}