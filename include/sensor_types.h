/**
 * @file sensor_types.h
 * @brief Datenstrukturen für alle Sensoren des InspectAir-Projekts
 * @author Team InspectAir
 * @date Januar 2026
 * 
 * Definiert die Datenstrukturen für:
 * - AHT20 (Temperatur/Luftfeuchtigkeit)
 * - SGP40 (VOC-Index)
 * - PMS5003 (Feinstaub)
 * - MH-Z19C (CO2)
 * - LD2410C (Radar/Präsenz)
 */

#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>

// ============================================
// SENSOR DATENSTRUKTUREN
// ============================================

/**
 * @brief Messdaten des AHT20 Temperatur-/Feuchtigkeitssensors
 */
typedef struct {
  float temperature;  /**< Temperatur in °C */
  float humidity;     /**< Relative Luftfeuchtigkeit in % */
} AHT20_Data;

/**
 * @brief Messdaten des SGP40 VOC-Sensors
 */
typedef struct {
  int32_t voc_index;  /**< VOC-Index (0-500, höher = schlechter) */
  uint16_t raw_value; /**< Rohwert des Sensors */
} SGP40_Data;

/**
 * @brief Messdaten des PMS5003 Feinstaubsensors
 * 
 * Enthält sowohl atmosphärische (AE) als auch Standard-Partikelwerte (SP)
 * für PM1.0, PM2.5 und PM10.0
 */
typedef struct {
  uint16_t PM_AE_UG_1_0;   /**< PM1.0 atmosphärisch in µg/m³ */
  uint16_t PM_AE_UG_2_5;   /**< PM2.5 atmosphärisch in µg/m³ */
  uint16_t PM_AE_UG_10_0;  /**< PM10.0 atmosphärisch in µg/m³ */
  uint16_t PM_SP_UG_1_0;   /**< PM1.0 Standard in µg/m³ */
  uint16_t PM_SP_UG_2_5;   /**< PM2.5 Standard in µg/m³ */
  uint16_t PM_SP_UG_10_0;  /**< PM10.0 Standard in µg/m³ */
} PMS5003_Data;

/**
 * @brief Messdaten des MH-Z19C CO2-Sensors
 */
typedef struct {
  int32_t co2_ppm;  /**< CO2-Konzentration in ppm */
  bool valid;       /**< true wenn Messung gültig */
} MHZ19C_Data;

/**
 * @brief Messdaten des LD2410C Radar-Sensors
 */
typedef struct {
  uint8_t presence;   /**< Präsenz erkannt (0/1) */
  uint16_t distance;  /**< Entfernung in cm */
  uint8_t motion;     /**< Bewegung erkannt (0/1) */
} LD2410C_Data;

/**
 * @brief Aggregierte Sensordaten aller Sensoren
 * 
 * Fasst alle Einzelsensor-Daten zusammen mit Zeitstempel
 */
typedef struct {
  AHT20_Data aht;       /**< Temperatur/Feuchte-Daten */
  SGP40_Data sgp;       /**< VOC-Daten */
  PMS5003_Data pms;     /**< Feinstaub-Daten */
  MHZ19C_Data mhz;      /**< CO2-Daten */
  LD2410C_Data radar;   /**< Radar-Daten */
  uint32_t timestamp;   /**< Zeitstempel der Messung (millis) */
} SensorReadings;

#endif
