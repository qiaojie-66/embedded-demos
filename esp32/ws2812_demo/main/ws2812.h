#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ws2812_init(uint8_t gpio_num, uint16_t num_leds);
void     ws2812_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void     ws2812_refresh(void);
void     ws2812_clear(void);

#ifdef __cplusplus
}
#endif
