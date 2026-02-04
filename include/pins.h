/**
 * @file pins.h
 * @brief Pin-Definitionen für das InspectAir-Projekt (ESP32-S3)
 * @author Team InspectAir
 * @date Januar 2026
 * 
 * Enthält alle GPIO-Zuordnungen für:
 * - Display (ST7796S via SPI)
 * - I2C-Sensoren (AHT20, SGP40)
 * - UART-Sensoren (PMS5003, MH-Z19C, LD2410C)
 * - UI-Navigation
 */

#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

// ============================================
// DISPLAY ST7796S (SPI)
// ============================================
/** @name Display-Pins (SPI)
 *  ST7796S 480x320 Display über SPI-Bus
 *  @{
 */
#define PIN_TFT_MOSI 11  /**< SPI Master Out Slave In */
#define PIN_TFT_MISO 13  /**< SPI Master In Slave Out */
#define PIN_TFT_SCLK 12  /**< SPI Clock */
#define PIN_TFT_CS   14  /**< Chip Select */
#define PIN_TFT_DC   9   /**< Data/Command */
#define PIN_TFT_RST  46  /**< Reset */
#define PIN_TFT_BL   3   /**< Backlight PWM */
/** @} */

// ============================================
// I2C SENSOREN (AHT20, SGP40)
// ============================================
/** @name I2C-Pins
 *  Für AHT20 (Temp/Feuchte) und SGP40 (VOC)
 *  @{
 */
#define PIN_I2C_SDA  8   /**< I2C Data */
#define PIN_I2C_SCL  18  /**< I2C Clock */
/** @} */

// ============================================
// UART SENSOREN
// ============================================
/** @name UART-Pins für Sensoren
 *  PMS5003 (Feinstaub), MH-Z19C (CO2), LD2410C (Radar)
 *  @{
 */
#define PIN_PMS_RX   16  /**< PMS5003 RX (ESP empfängt) */
#define PIN_PMS_TX   17  /**< PMS5003 TX (ESP sendet) */
#define PIN_CO2_RX   4   /**< MH-Z19C RX */
#define PIN_CO2_TX   5   /**< MH-Z19C TX */
#define PIN_RADAR_RX 7   /**< LD2410C RX */
#define PIN_RADAR_TX 6   /**< LD2410C TX */
#define PIN_RADAR_OUT 15 /**< LD2410C OUT (Digital-Ausgang) */
/** @} */

// ============================================
// UI NAVIGATION
// ============================================
/** @name UI-Steuerung
 *  Button für Screen-Wechsel
 *  @{
 */
#define PIN_UI_BUTTON  1  /**< UI-Button (Active LOW, interner Pullup) */
/** @} */

#endif