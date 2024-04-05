# STM32 to ESP32 UART Comms
A short project used to showcase how one can use the STM32's capablities in conunction with the ESP32's IoT ability.

## Hardware used
STM32F401RE Nucleo-64

ESP32-S3-DevKit-C

ADS1117 16-bit ADC

## Software
### STM32F401RE
The firmware for this was written in bare-metal C, meaning that it will only work in the environment I currently have set up, however, the idea would be the same regardless. 

### ESP32
I used the ESP-IDF for this so all youd need to do is copy the main file, set the target board to yours, and flash it. 

## What does it do?
The STM32 gathers readings from an ADS1117 ADC, which is a 4 channel ADC. These readings are then concatenated into one string, then sent over to the ESP32 through UART. On the ESP32 side, a task is used to recieve the entire data stream, once this stream has been received another task is used to decode the string. The decoding is done in the order the readings are sent, which allows for as many readings to be sent over, decoded, and sent over through Wi-Fi or Bluetooth. 