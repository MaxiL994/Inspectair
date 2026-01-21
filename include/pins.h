#ifndef PINS_H
#define PINS_H

// ============================================
// PIN DEFINITIONEN - INSPECTAIR
// ============================================

// === SPI DISPLAY (ST7796S) ===
#define PIN_TFT_MOSI    11
#define PIN_TFT_MISO    13
#define PIN_TFT_SCLK    12
#define PIN_TFT_CS      14
#define PIN_TFT_DC      9
#define PIN_TFT_RST     46
#define PIN_TFT_BL      3

// === I²C BUS (AHT20 + SGP40) ===
#define PIN_I2C_SDA     8
#define PIN_I2C_SCL     18

// === UART1 - PMS5003 (Feinstaub) ===
#define PIN_PMS_RX      16   // ESP32 RX (Sensor TX)
#define PIN_PMS_TX      17   // ESP32 TX (Sensor RX)

// === UART2 - MH-Z19C (CO2) ===
#define PIN_CO2_RX      4    // ESP32 RX (Sensor TX)
#define PIN_CO2_TX      5    // ESP32 TX (Sensor RX)

// === LD2410C (Radar) - über Level Shifter ===
#define PIN_RADAR_RX    7    // Direkt vom Sensor
#define PIN_RADAR_TX    6    // Vom Level Shifter
#define PIN_RADAR_OUT   15   // Presence Detection

// === BUTTON ===
#define PIN_BUTTON      0    // UI-Umschaltung (mit internem Pull-up)

#endif
