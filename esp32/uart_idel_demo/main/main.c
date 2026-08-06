#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

static uint8_t uart_buffer[1024];

static QueueHandle_t uart_queue;

void app_main(void)
{
   
   uart_event_t uart_event;
   uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, GPIO_NUM_40, GPIO_NUM_41, -1, -1);

    uart_driver_install(UART_NUM_0, 1024, 1024, 20, &uart_queue, 0);
    while(1)
    {
        if(pdTRUE == xQueueReceive(uart_queue, &uart_event, portMAX_DELAY))
        {
            switch(uart_event.type)
            {
                case UART_DATA:
                    ESP_LOGI("UART", "UART_DATA, size: %d", uart_event.size);
                    uart_read_bytes(UART_NUM_0, uart_buffer, uart_event.size, pdMS_TO_TICKS(1000));
                    uart_write_bytes(UART_NUM_0, uart_buffer, uart_event.size);
                    break;
                case UART_BUFFER_FULL:

                    break;
                case UART_FIFO_OVF:

                    break;
                default:break;
            }
        }
    }
    
}
