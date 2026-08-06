#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
static EventGroupHandle_t ledc_event_group;
#define LEDC_FULL_BIT  BIT0
#define LEDC_EMPTY_BIT BIT1
bool IRAM_ATTR ledc_finish(const ledc_cb_param_t *param, void *user_arg)
{
    BaseType_t taskWoken;
    if(param->duty)
    {
        xEventGroupSetBitsFromISR(ledc_event_group, LEDC_FULL_BIT, &taskWoken);
    }
    else
    {
        xEventGroupSetBitsFromISR(ledc_event_group, LEDC_EMPTY_BIT, &taskWoken);
    }
    return taskWoken == pdTRUE ? true : false;
}

void led_run(void *param)
{
    EventBits_t uxBits;
    while(1)
    {
        uxBits = xEventGroupWaitBits(ledc_event_group, LEDC_FULL_BIT | LEDC_EMPTY_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if(uxBits & LEDC_FULL_BIT)
        {
            // Handle LEDC full event
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 2000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
        }
        else if(uxBits & LEDC_EMPTY_BIT)
        {
            // Handle LEDC empty event
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191, 2000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
        }
    }
}

void app_main(void)
{
    gpio_config_t led_cfg =
    {
        .pin_bit_mask = (1 << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_cfg);

    ledc_timer_config_t ledc_timer_bit_t =//timer负责频率+分辨率
    {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer_bit_t);

    ledc_channel_config_t ledc_channel =//channel负责输出引脚+占空比
    {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .intr_type= LEDC_INTR_DISABLE,
        .gpio_num = GPIO_NUM_2
    };
    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191, 2000);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);

    ledc_event_group = xEventGroupCreate();

    ledc_cbs_t fade_cb = {
        .fade_cb = ledc_finish
    };
    ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &fade_cb, NULL);

    xTaskCreate(led_run, "led_run", 2048, NULL, 5, NULL);
}
