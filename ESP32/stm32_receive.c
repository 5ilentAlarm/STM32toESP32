/*
    Understanding the basics of RTOS through blinking an LED
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "string.h"
#include <ctype.h>
#include "freertos/semphr.h"

#define BLINK_LED 4
static const int RX_BUF_SIZE = 1024;
static const int RX_BUF_SIZE_U1 = 1024;
uint32_t time = 500; //delay

#define TXD_PIN (GPIO_NUM_43)
#define RXD_PIN (GPIO_NUM_44)

#define U1_TXD_PIN (GPIO_NUM_17)
#define U1_RXD_PIN (GPIO_NUM_18)

#define num_readings 20

//mutex
static SemaphoreHandle_t mutex; //For Flag

//Global array to hold values
char * uartData = "\0";

//To switch tasks
bool dataReceived = 0;

//initialize UART ports, as well as GPIO ports
void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    //Install and initialize uart for printf(uart0)
    uart_driver_install(UART_NUM_0, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Do the same but for UART1, this will read data from the STM32
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE_U1 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, U1_TXD_PIN, U1_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void vTask_recieve(void *param) {
    static const char *currTaskName = "RX task\n";
    ESP_LOGI(currTaskName, "Starting!\n");
    uint8_t* data = (uint8_t*)malloc(RX_BUF_SIZE_U1 + 1); //allocate the size of our buffer into a data pointer, this will hold our receieving data
    for(;;) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE_U1, 1 / portTICK_PERIOD_MS); 
        if(rxBytes > 0) {
            data[rxBytes] = 0;  //set the last character to zero? 
            uartData = (char*)data;
            printf("Received: %s\n", data);
        }
        xSemaphoreTake(mutex, portMAX_DELAY);
        dataReceived = 1;
        xSemaphoreGive(mutex);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    free(data);
    vTaskDelete(NULL);
}

static void vTask_outputArray(void *param){
    for(;;){
        if(dataReceived){
            xSemaphoreTake(mutex, portMAX_DELAY);
            char *parseData = "\0";
            while(*uartData){
                if(isdigit((int)*uartData) || ((*uartData == '-' || *uartData == '+') && isdigit((int)*(uartData+1)))) {
                    long val = strtol(uartData, &parseData, 10);
                    printf("%ld is the parsed value\n", val);
                    uartData = parseData;
                }else {
                    uartData++;
                }
            }
            dataReceived = 0;
            xSemaphoreGive(mutex);
        }
     }
     vTaskDelete(NULL);
}

void app_main(void)
{
    char *ourTaskName = pcTaskGetName(NULL);
    ESP_LOGI(ourTaskName, "Hello, starting up!\n");
    init();

    mutex = xSemaphoreCreateMutex();

    xTaskCreate(&vTask_outputArray, "Output_array", 2048 * 2, NULL, tskIDLE_PRIORITY, NULL); //set to same priority 
    xTaskCreate(&vTask_recieve, "Rx_Task", 2048 * 2, NULL, tskIDLE_PRIORITY, NULL);
}