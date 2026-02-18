# PIN Mapping (ESP32-S3)

## Display ST7796S (SPI)
| Pin | GPIO | Funktion |
|-----|------|----------|
| TFT_MOSI | 11 | SPI Master Out Slave In |
| TFT_MISO | 13 | SPI Master In Slave Out |
| TFT_SCLK | 12 | SPI Clock |
| TFT_CS | 14 | Chip Select |
| TFT_DC | 9 | Data/Command |
| TFT_RST | 46 | Reset |
| TFT_BL | 3 | Backlight PWM |

## I2C Sensoren (AHT20, SGP40)
| Pin | GPIO | Funktion |
|-----|------|----------|
| I2C_SDA | 8 | I2C Data |
| I2C_SCL | 18 | I2C Clock |

## UART Sensoren
| Pin | GPIO | Funktion |
|-----|------|----------|
| PMS_RX | 16 | PMS5003 RX (ESP empfängt) |
| PMS_TX | 17 | PMS5003 TX (ESP sendet) |
| CO2_RX | 4 | MH-Z19C RX |
| CO2_TX | 5 | MH-Z19C TX |
| RADAR_RX | 7 | LD2410C RX |
| RADAR_TX | 6 | LD2410C TX |
| RADAR_OUT | 15 | LD2410C OUT (Digitalausgang) |

## UI Navigation
| Pin | GPIO | Funktion |
|-----|------|----------|
| UI_BUTTON | 1 | UI Button (active LOW, interner Pullup) |

## Display Dimming
| Pin | GPIO | Funktion |
|-----|------|----------|
| DISPLAY_DIM | 21 | Display Dimming MOSFET (PWM) |
