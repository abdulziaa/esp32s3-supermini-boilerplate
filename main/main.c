/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "led_strip.h"

#include "buzzer.h"

// if running on pasta, define PASTA
#define PASTA 0

// Numbers of the LED in the strip
#define LED_STRIP_LED_COUNT 1
#define LED_STRIP_MEMORY_BLOCK_WORDS 1024 // this determines the DMA block size

// GPIO assignment
#define LED_STRIP_GPIO_PIN  48

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

static const char *TAG = "example";

led_strip_handle_t configure_led(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO_PIN, // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_COUNT,      // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812,        // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color order of the strip: GRB
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .mem_block_symbols = LED_STRIP_MEMORY_BLOCK_WORDS, // the memory block size used by the RMT channel
        .flags = {
            .with_dma = 1,     // Using DMA can improve performance when driving more LEDs
        }
    };

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    return led_strip;
}

TaskHandle_t rgb_task_handle;
void rgb_task(void *pvParameter)
{
    led_strip_handle_t led_strip = configure_led();
    uint8_t red = 255, green = 0, blue = 0;

    while (1) {
        /* Set the LED pixel using RGB, then refresh */
        for (int i = 0; i < LED_STRIP_LED_COUNT; i++) {
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
        }
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));

        /* Cycle through colors: R -> G -> B -> R */
        if (red > 0 && blue == 0) {
            red--;
            green++;
        } else if (green > 0 && red == 0) {
            green--;
            blue++;
        } else if (blue > 0 && green == 0) {
            blue--;
            red++;
        }

        vTaskDelay(pdMS_TO_TICKS(3));
    }
}

TaskHandle_t buzzer_task_handle;
void buzzer_task(void *pvParameters)
{
    buzzer_init(41);
    while (1) 
    {
        // Play a beautiful ascending melody
        buzzer(NOTE_C4, 128, 200);
        buzzer(NOTE_E4, 128, 200);
        buzzer(NOTE_G4, 128, 200);
        buzzer(NOTE_C5, 128, 400);
        
        // Descending notes
        buzzer(NOTE_G4, 128, 200);
        buzzer(NOTE_E4, 128, 200);
        buzzer(NOTE_C4, 128, 400);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}    


void app_main(void)
{
    ESP_LOGI(TAG, "Start tasks");
    xTaskCreate(rgb_task, "rgb_task", 2048, NULL, 5, &rgb_task_handle);

    #if PASTA
    xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, 5, &buzzer_task_handle);
    #endif
}