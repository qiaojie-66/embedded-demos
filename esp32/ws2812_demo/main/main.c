#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ws2812.h"

static const char *TAG = "main";

#define WS2812_GPIO  48
#define WS2812_NUM   1    /* 1 颗灯珠 */

void app_main(void)
{
    ws2812_init(WS2812_GPIO, WS2812_NUM);

    uint8_t r = 255, g = 0, b = 0;

    while (1) {
        /* RGB 三色渐变 */
        if      (r == 255 && g < 255 && b == 0)   g += 5;    /* 红 -> 黄 */
        else if (r > 0   && g == 255 && b == 0)   r -= 5;    /* 黄 -> 绿 */
        else if (r == 0   && g == 255 && b < 255)  b += 5;   /* 绿 -> 青 */
        else if (r == 0   && g > 0   && b == 255)  g -= 5;   /* 青 -> 蓝 */
        else if (r < 255 && g == 0   && b == 255)  r += 5;   /* 蓝 -> 紫 */
        else if (r == 255 && g == 0   && b > 0)    b -= 5;   /* 紫 -> 红 */

        ws2812_set_pixel(0, r, g, b);
       
        ws2812_refresh();
        ESP_LOGI(TAG, "R=%d G=%d B=%d", r, g, b);
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
