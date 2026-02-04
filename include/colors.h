/**
 * @file colors.h
 * @brief Color definitions and threshold functions for the InspectAir project
 * @author Team InspectAir
 * @date January 2026
 * 
 * Defines RGB565 colors and classification functions for:
 * - CO2 levels (WHO guidelines)
 * - PM2.5 particulate matter (WHO 2021)
 * - VOC index
 * - Temperature (comfort zone)
 * - Humidity (comfort zone)
 */

#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

// ============================================
// COLOR DEFINITIONS (RGB565)
// ============================================

/** @name Primary colors for status display
 *  @{
 */
#define COLOR_GOOD       0x07E0  /**< Green - Good air quality */
#define COLOR_MODERATE   0xFFE0  /**< Yellow - Moderate air quality */
#define COLOR_BAD        0xFD20  /**< Orange - Poor air quality */
#define COLOR_DANGER     0xF800  /**< Red - Dangerous air quality */
/** @} */

/** @name Background and text colors
 *  @{
 */
#define COLOR_BG         0x0000  /**< Black (background) */
#define COLOR_TEXT       0xFFFF  /**< White (main text) */
#define COLOR_LABEL      0xAD55  /**< Gray (labels) */
#define COLOR_BOX_BG     0x1082  /**< Dark blue-gray (tiles) */
#define COLOR_TITLE      0x07FF  /**< Cyan (title) */
/** @} */

// ============================================
// THRESHOLDS (WHO guidelines / Comfort zone)
// ============================================

/** @name CO2 thresholds (ppm)
 *  @{
 */
#define LIMIT_CO2_GOOD     800   /**< Below 800 ppm = good */
#define LIMIT_CO2_MODERATE 1000  /**< 800-1000 ppm = moderate */
#define LIMIT_CO2_BAD      1500  /**< 1000-1500 ppm = poor */
/** @} */

/** @name PM2.5 thresholds (µg/m³, WHO 2021)
 *  @{
 */
#define LIMIT_PM25_GOOD     5    /**< WHO 2021: ≤5 µg/m³ = good */
#define LIMIT_PM25_MODERATE 15   /**< WHO 2021: ≤15 µg/m³ = moderate */
#define LIMIT_PM25_BAD      25   /**< WHO 2021: ≤25 µg/m³ = poor */
/** @} */

/** @name VOC index thresholds
 *  @{
 */
#define LIMIT_VOC_GOOD     100   /**< ≤100 = good */
#define LIMIT_VOC_MODERATE 200   /**< 100-200 = moderate */
#define LIMIT_VOC_BAD      300   /**< 200-300 = poor */
/** @} */

/** @name Temperature thresholds (°C, comfort zone)
 *  @{
 */
#define LIMIT_TEMP_GOOD_MIN  18  /**< Lower limit comfort zone */
#define LIMIT_TEMP_GOOD_MAX  26  /**< Upper limit comfort zone */
#define LIMIT_TEMP_WARN_MIN  15  /**< Lower limit acceptable */
#define LIMIT_TEMP_WARN_MAX  30  /**< Upper limit acceptable */
/** @} */

/** @name Humidity thresholds (%, comfort zone)
 *  @{
 */
#define LIMIT_HUM_GOOD_MIN  30   /**< Lower limit comfort zone */
#define LIMIT_HUM_GOOD_MAX  60   /**< Upper limit comfort zone */
#define LIMIT_HUM_WARN_MIN  20   /**< Lower limit acceptable */
#define LIMIT_HUM_WARN_MAX  70   /**< Upper limit acceptable */
/** @} */

// ============================================
// COLOR CLASSIFICATION - SENSORS
// ============================================

/**
 * @brief Determines color based on CO2 level
 * @param co2 CO2 concentration in ppm
 * @return RGB565 color (COLOR_GOOD/MODERATE/BAD/DANGER)
 */
inline uint16_t getCO2Color(int co2) {
  if (co2 < LIMIT_CO2_GOOD)     return COLOR_GOOD;
  if (co2 < LIMIT_CO2_MODERATE) return COLOR_MODERATE;
  if (co2 < LIMIT_CO2_BAD)      return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * @brief Determines color based on PM2.5 level (WHO 2021)
 * @param pm25 Particulate matter in µg/m³
 * @return RGB565 color (COLOR_GOOD/MODERATE/BAD/DANGER)
 */
inline uint16_t getPM25Color(int pm25) {
  if (pm25 <= LIMIT_PM25_GOOD)     return COLOR_GOOD;
  if (pm25 <= LIMIT_PM25_MODERATE) return COLOR_MODERATE;
  if (pm25 <= LIMIT_PM25_BAD)      return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * @brief Determines color based on VOC index
 * @param voc VOC index (0-500)
 * @return RGB565 color (COLOR_GOOD/MODERATE/BAD/DANGER)
 */
inline uint16_t getVOCColor(int voc) {
  if (voc <= LIMIT_VOC_GOOD)     return COLOR_GOOD;
  if (voc <= LIMIT_VOC_MODERATE) return COLOR_MODERATE;
  if (voc <= LIMIT_VOC_BAD)      return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * @brief Determines color based on temperature
 * @param temp Temperature in °C
 * @return RGB565 color (COLOR_GOOD/MODERATE/BAD)
 */
inline uint16_t getTempColor(float temp) {
  if (temp >= LIMIT_TEMP_GOOD_MIN && temp <= LIMIT_TEMP_GOOD_MAX) return COLOR_GOOD;
  if (temp >= LIMIT_TEMP_WARN_MIN && temp <= LIMIT_TEMP_WARN_MAX) return COLOR_MODERATE;
  return COLOR_BAD;
}

/**
 * @brief Determines color based on humidity
 * @param hum Relative humidity in %
 * @return RGB565 color (COLOR_GOOD/MODERATE/BAD)
 */
inline uint16_t getHumColor(float hum) {
  if (hum >= LIMIT_HUM_GOOD_MIN && hum <= LIMIT_HUM_GOOD_MAX) return COLOR_GOOD;
  if (hum >= LIMIT_HUM_WARN_MIN && hum <= LIMIT_HUM_WARN_MAX) return COLOR_MODERATE;
  return COLOR_BAD;
}

#endif
