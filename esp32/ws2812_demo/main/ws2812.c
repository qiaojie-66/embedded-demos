#include "ws2812.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "ws2812";

#define RMT_HZ  10000000   /* 10MHz, 1 tick = 100ns */
#define T0H     4           /* 0 码高电平 400ns */
#define T0L     8           /* 0 码低电平 800ns */
#define T1H     8           /* 1 码高电平 800ns */
#define T1L     4           /* 1 码低电平 400ns */
#define RST     600         /* 复位 >50µs */

static rmt_channel_handle_t  tx_chan  = NULL;
static rmt_encoder_handle_t  copy_enc = NULL;
static uint8_t              *pixels   = NULL;
static uint16_t              num      = 0;

/* 每个 bit 对应一个 RMT 符号（高+低） */
static const rmt_symbol_word_t bit0 = {
    .duration0 = T0H, .level0 = 1,
    .duration1 = T0L, .level1 = 0,
};
static const rmt_symbol_word_t bit1 = {
    .duration0 = T1H, .level0 = 1,
    .duration1 = T1L, .level1 = 0,
};

esp_err_t ws2812_init(uint8_t gpio_num, uint16_t num_leds)
{
    num = num_leds;
    pixels = calloc(num, 3);  /* GRB, 3 字节/LED */
    if (!pixels) return ESP_ERR_NO_MEM;

    rmt_tx_channel_config_t cfg = {
        .gpio_num           = gpio_num,
        .clk_src            = RMT_CLK_SRC_DEFAULT,
        .resolution_hz      = RMT_HZ,
        .mem_block_symbols  = 64,
        .trans_queue_depth  = 4,
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&cfg, &tx_chan), TAG, "创建 RMT 通道失败");
    ESP_RETURN_ON_ERROR(rmt_enable(tx_chan), TAG, "RMT 使能失败");

    rmt_copy_encoder_config_t copy_cfg = {};
    ESP_RETURN_ON_ERROR(rmt_new_copy_encoder(&copy_cfg, &copy_enc), TAG, "创建 encoder 失败");

    ESP_LOGI(TAG, "初始化完成, GPIO=%d, LED 数量=%d", gpio_num, num_leds);
    ws2812_clear();
    return ESP_OK;
}

void ws2812_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= num) return;
    pixels[index * 3 + 0] = g;   /* GRB 顺序 */
    pixels[index * 3 + 1] = r;
    pixels[index * 3 + 2] = b;
}

void ws2812_refresh(void)
{
    if (!tx_chan || !copy_enc || !pixels || num == 0) return;

    uint16_t item_count = (uint16_t)num * 24 + 1;  /* 24bit/LED + 复位码 */
    size_t buf_size = item_count * sizeof(rmt_symbol_word_t);
    rmt_symbol_word_t *buf = malloc(buf_size);
    if (!buf) return;

    /* 将像素数据编码为 RMT 符号 */
    uint16_t idx = 0;
    for (uint16_t led = 0; led < num; led++) {
        uint8_t *p = &pixels[led * 3];  /* G, R, B */
        for (int byte = 0; byte < 3; byte++) {
            for (int bit = 7; bit >= 0; bit--) {   /* MSB 优先 */
                buf[idx++] = (p[byte] & (1 << bit)) ? bit1 : bit0;
            }
        }
    }
    /* 复位码 */
    buf[idx] = (rmt_symbol_word_t){ .duration0 = RST, .level0 = 0,
                                     .duration1 = 0,  .level1 = 0 };

    rmt_transmit_config_t tx = { .loop_count = 0 };
    rmt_transmit(tx_chan, copy_enc, buf, buf_size, &tx);
    rmt_tx_wait_all_done(tx_chan, portMAX_DELAY);
    free(buf);
}

void ws2812_clear(void)
{
    if (!pixels) return;
    memset(pixels, 0, num * 3);
    ws2812_refresh();
}
