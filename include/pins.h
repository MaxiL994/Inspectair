#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

// Display ST7796S (SPI)
#define PIN_TFT_MOSI 11
#define PIN_TFT_MISO 13
#define PIN_TFT_SCLK 12
#define PIN_TFT_CS   14
#define PIN_TFT_DC   9
#define PIN_TFT_RST  46
#define PIN_TFT_BL   3

// I2C Sensoren (AHT20, SGP40)
#define PIN_I2C_SDA  8
#define PIN_I2C_SCL  18

// UART Sensoren
#define PIN_PMS_RX   16
#define PIN_PMS_TX   17
#define PIN_CO2_RX   4
#define PIN_CO2_TX   5
#define PIN_RADAR_RX 7
#define PIN_RADAR_TX 6
#define PIN_RADAR_OUT 15

// UI Navigation Button (Active LOW mit internem Pullup)
// GPIO 1 (frei)
#define PIN_UI_BUTTON  1

#endif