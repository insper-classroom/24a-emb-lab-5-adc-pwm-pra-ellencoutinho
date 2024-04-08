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


QueueHandle_t xQueueAdc;
const int ADC_X = 28;
const int ADC_X_ID = 2;
const int ADC_Y = 27;
const int ADC_Y_ID = 1;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void uart_task(void *p) {
    adc_t data;
    // Set up our UART with the required speed.

    while (1) {
        if (xQueueReceive(xQueueAdc, &data, portMAX_DELAY)) {
            int val = data.val;
            int msb = val >> 8;
            int lsb = val & 0xFF ;

            uart_putc_raw(uart0, data.axis);
            uart_putc_raw(uart0, lsb);
            uart_putc_raw(uart0, msb);
            uart_putc_raw(uart0, -1);
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
    
    // Select ADC input 1 (GPIO27)
    

    int valores[5] = {0,0,0,0,0};
    adc_t data;

    while (1) {
        adc_gpio_init(ADC_X);
        adc_select_input(ADC_X_ID);
        int valor_analogico_x = converte_valor(adc_read()); // Lê o valor analógico do pino ADC

        for(int k=0;k<4;k++){
            valores[k]=valores[k+1];
        }
        valores[4]=valor_analogico_x;

        int soma = 0;
        for(int j=0; j<5; j++){
            soma += valores[j];
        }
        int m=soma/5;

        data.axis = 0;
        data.val = m;
       // printf("Valor analógico X: %d e %d\n", m, valor_analogico_x);
        xQueueSend(xQueueAdc, &data, 0);

        vTaskDelay(pdMS_TO_TICKS(500));
        
    }
}   

void y_task(void *p) {
    adc_init();
    
    // Select ADC input 1 (GPIO27)
    
    adc_t data;
    int valores[5] = {0,0,0,0,0};


    while (1) {
        adc_gpio_init(ADC_Y);
        adc_select_input(ADC_Y_ID);
        int valor_analogico_y = converte_valor(adc_read()); // Lê o valor analógico do pino ADC

        for(int k=0;k<4;k++){
            valores[k]=valores[k+1];
        }
        valores[4]=valor_analogico_y;

        int soma = 0;
        for(int j=0; j<5; j++){
            soma += valores[j];
        }
        int m=soma/5;

        data.axis = 1;
        data.val = m;
        xQueueSend(xQueueAdc, &data, 0);

        //printf("Valor analógico Y: %d e %d \n", m, valor_analogico_y);

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
