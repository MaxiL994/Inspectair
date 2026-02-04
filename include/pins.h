/**
 * @file pins.h
 * @brief Pin definitions for the InspectAir project (ESP32-S3)
 * @author Team InspectAir
 * @date January 2026
 * 
 * Contains all GPIO assignments for:
 * - Display (ST7796S via SPI)
 * - I2C sensors (AHT20, SGP40)
 * - UART sensors (PMS5003, MH-Z19C, LD2410C)
 * - UI navigation
 */

#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

// ============================================
// DISPLAY ST7796S (SPI)
// ============================================
/** @name Display pins (SPI)
 *  ST7796S 480x320 display via SPI bus
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
// I2C SENSORS (AHT20, SGP40)
// ============================================
/** @name I2C pins
 *  For AHT20 (temp/humidity) and SGP40 (VOC)
 *  @{
 */
#define PIN_I2C_SDA  8   /**< I2C Data */
#define PIN_I2C_SCL  18  /**< I2C Clock */
/** @} */

// ============================================
// UART SENSORS
// ============================================
/** @name UART pins for sensors
 *  PMS5003 (particulate), MH-Z19C (CO2), LD2410C (radar)
 *  @{
 */
#define PIN_PMS_RX   16  /**< PMS5003 RX (ESP receives) */
#define PIN_PMS_TX   17  /**< PMS5003 TX (ESP sends) */
#define PIN_CO2_RX   4   /**< MH-Z19C RX */
#define PIN_CO2_TX   5   /**< MH-Z19C TX */
#define PIN_RADAR_RX 7   /**< LD2410C RX */
#define PIN_RADAR_TX 6   /**< LD2410C TX */
#define PIN_RADAR_OUT 15 /**< LD2410C OUT (digital output) */
/** @} */

// ============================================
// UI NAVIGATION
// ============================================
/** @name UI control
 *  Button for screen switching
 *  @{
 */
#define PIN_UI_BUTTON  1  /**< UI button (active LOW, internal pullup) */
/** @} */

#endif