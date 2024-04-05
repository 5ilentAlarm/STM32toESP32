#include "stm32f401re.h"
#include "lib/ADS1115.h"
#include "lib/MCP23017.h"
#include <string.h>

#define BAUD 115200

static volatile uint32_t s_ticks;
void SysTick_Handler(void) {
  s_ticks++;
}

uint32_t SystemCoreClock = FREQ;
void SystemInit(void) {
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;    // Enable SYSCFG
  SysTick_Config(SystemCoreClock / 1000);  // Tick every 1 ms
}
 
uint32_t millis(void) {
    return s_ticks;
}

static inline void delay(uint32_t time) {
    uint32_t currentTime = millis();
    while(currentTime + time > s_ticks) { //if we want to delay for 500ms, current s_ticks is 20, we want to wait untill 520ms
        //do nothing
    }
}

int main(void) {
  uart2_init(BAUD); //enable uart for printf
  uart1_init(BAUD);
  delay(1);
  printf("Uart1 Initialized\r\n");
  //uartWrite_buf(UART1, "do\r\n", 4); //add 2 for carriage return and newline
  i2c1_init();
  printf("I2C init finished\r\n");

  ADS1115_Init(ADS1115_DEFAULT_ADDR, ADS1115_REG_CONFIG, 0xB3, ADS1115_DEFAULT_CONFIG_LSB);
  printf("ADS1115 Configured!\r\n");

  ADS1115_setConvReg(ADS1115_DEFAULT_ADDR, ADS1115_REG_CONVERSION); //set register pointer to the conversion register
  unsigned int data = ADS1115_getSamples(ADS1115_DEFAULT_ADDR, 100);
  printf("Current Voltage received: %d\r\n", data);
  char s[10];
  itoa((int)data, s, 10);
  printf("Converted reading: %s\r\n", s);
  int temp = 555;
  char s1[10];
  itoa(temp, s1, 10);
  size_t len = strlen(s);
  size_t len2 = strlen(s1);

  for (;;) {
    uartWrite_buf(UART1, "ADC Reading: ", 13);
    uartWrite_buf(UART1, s, len);
    uartWrite_buf(UART1, "Second Reading: ", 16);
    uartWrite_buf(UART1, s1, len2);
    delay(500);
  }
  return 0;
}

