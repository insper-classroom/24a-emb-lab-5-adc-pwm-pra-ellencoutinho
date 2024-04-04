/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

QueueHandle_t xQueueAdc;
const int ADC_X = 28;
const int ADC_X_ID = 2;
const int ADC_Y = 27;
const int ADC_Y_ID = 1;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void write_package(adc_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF ;

    uart_putc_raw(uart0, data.axis);
    uart_putc_raw(uart0, lsb);
    uart_putc_raw(uart0, msb);
    uart_putc_raw(uart0, -1);
}

void uart_task(void *p) {
    adc_t data;
    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (1) {
        if (xQueueReceive(xQueueAdc, &data, portMAX_DELAY)) {
            write_package(data);

        }
    }
}

int converte_valor(int valor_analogico){
    int valor_convertido = (valor_analogico * 510) / 4095 - 255;
    if(valor_convertido>-30 && valor_convertido<30){
        return 0;
    }
    return valor_convertido;
};

void x_task(void *p) {
    adc_init();
    adc_gpio_init(ADC_X);
    // Select ADC input 1 (GPIO27)
    adc_select_input(ADC_X_ID);

    adc_t data;

    while (1) {
        int valor_analogico_x = converte_valor(adc_read()); // Lê o valor analógico do pino ADC
        printf("Valor analógico X: %d\n", valor_analogico_x);
        data.axis = 0;
        data.val = valor_analogico_x;
        xQueueSend(xQueueAdc, &data, 0);

        vTaskDelay(pdMS_TO_TICKS(500));
        
    }
}   

void y_task(void *p) {
    adc_init();
    adc_gpio_init(ADC_Y);
    // Select ADC input 1 (GPIO27)
    adc_select_input(ADC_Y_ID);
    adc_t data;


    while (1) {
        int valor_analogico_y = converte_valor(adc_read()); // Lê o valor analógico do pino ADC
        data.axis = 1;
        data.val = valor_analogico_y;
        xQueueSend(xQueueAdc, &data, 0);

        printf("Valor analógico Y: %d\n", valor_analogico_y);

        vTaskDelay(pdMS_TO_TICKS(500));
        
    }
}

int main() {
    stdio_init_all();   

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(x_task, "X", 256, NULL, 1, NULL);
    xTaskCreate(y_task, "Y", 256, NULL, 1, NULL);

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
