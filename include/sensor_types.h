/**
 * @file sensor_types.h
 * @brief Data structures for all sensors in the InspectAir project
 * @author Team InspectAir
 * @date January 2026
 * 
 * Defines data structures for:
 * - AHT20 (temperature/humidity)
 * - SGP40 (VOC index)
 * - PMS5003 (particulate matter)
 * - MH-Z19C (CO2)
 * - LD2410C (radar/presence)
 */

#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>

// ============================================
// SENSOR DATA STRUCTURES
// ============================================

/**
 * @brief Measurement data from AHT20 temperature/humidity sensor
 */
typedef struct {
  float temperature;  /**< Temperature in °C */
  float humidity;     /**< Relative humidity in % */
} AHT20_Data;

/**
 * @brief Measurement data from SGP40 VOC sensor
 */
typedef struct {
  int32_t voc_index;  /**< VOC index (0-500, higher = worse) */
  uint16_t raw_value; /**< Raw sensor value */
} SGP40_Data;

/**
 * @brief Measurement data from PMS5003 particulate sensor
 * 
 * Contains both atmospheric (AE) and standard particle values (SP)
 * for PM1.0, PM2.5 and PM10.0
 */
typedef struct {
  uint16_t PM_AE_UG_1_0;   /**< PM1.0 atmospheric in µg/m³ */
  uint16_t PM_AE_UG_2_5;   /**< PM2.5 atmospheric in µg/m³ */
  uint16_t PM_AE_UG_10_0;  /**< PM10.0 atmospheric in µg/m³ */
  uint16_t PM_SP_UG_1_0;   /**< PM1.0 standard in µg/m³ */
  uint16_t PM_SP_UG_2_5;   /**< PM2.5 standard in µg/m³ */
  uint16_t PM_SP_UG_10_0;  /**< PM10.0 standard in µg/m³ */
} PMS5003_Data;

/**
 * @brief Measurement data from MH-Z19C CO2 sensor
 */
typedef struct {
  int32_t co2_ppm;  /**< CO2 concentration in ppm */
  bool valid;       /**< true if measurement is valid */
} MHZ19C_Data;

/**
 * @brief Measurement data from LD2410C radar sensor
 */
typedef struct {
  uint8_t presence;   /**< Presence detected (0/1) */
  uint16_t distance;  /**< Distance in cm */
  uint8_t motion;     /**< Motion detected (0/1) */
} LD2410C_Data;

/**
 * @brief Aggregated sensor data from all sensors
 * 
 * Combines all individual sensor data with timestamp
 */
typedef struct {
  AHT20_Data aht;       /**< Temperature/humidity data */
  SGP40_Data sgp;       /**< VOC data */
  PMS5003_Data pms;     /**< Particulate data */
  MHZ19C_Data mhz;      /**< CO2 data */
  LD2410C_Data radar;   /**< Radar data */
  uint32_t timestamp;   /**< Timestamp of measurement (millis) */
} SensorReadings;

#endif
