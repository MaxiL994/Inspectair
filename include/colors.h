/**
 * @file colors.h
 * @brief Farbdefinitionen und Grenzwert-Funktionen für das InspectAir-Projekt
 * @author Team InspectAir
 * @date Januar 2026
 * 
 * Definiert RGB565-Farben und Klassifizierungsfunktionen für:
 * - CO2-Pegel (WHO-Richtwerte)
 * - PM2.5-Feinstaub (WHO 2021)
 * - VOC-Index
 * - Temperatur (Behaglichkeit)
 * - Luftfeuchtigkeit (Behaglichkeit)
 */

#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

// ============================================
// FARBDEFINITIONEN (RGB565)
// ============================================

/** @name Primärfarben für Status-Anzeige
 *  @{
 */
#define COLOR_GOOD       0x07E0  /**< Grün - Gute Luftqualität */
#define COLOR_MODERATE   0xFFE0  /**< Gelb - Mäßige Luftqualität */
#define COLOR_BAD        0xFD20  /**< Orange - Schlechte Luftqualität */
#define COLOR_DANGER     0xF800  /**< Rot - Gefährliche Luftqualität */
/** @} */

/** @name Hintergrund- und Textfarben
 *  @{
 */
#define COLOR_BG         0x0000  /**< Schwarz (Hintergrund) */
#define COLOR_TEXT       0xFFFF  /**< Weiß (Haupttext) */
#define COLOR_LABEL      0xAD55  /**< Grau (Labels) */
#define COLOR_BOX_BG     0x1082  /**< Dunkles Blaugrau (Kacheln) */
#define COLOR_TITLE      0x07FF  /**< Cyan (Titel) */
/** @} */

// ============================================
// GRENZWERTE (WHO-Richtwerte / Behaglichkeit)
// ============================================

/** @name CO2-Grenzwerte (ppm)
 *  @{
 */
#define LIMIT_CO2_GOOD     800   /**< Unter 800 ppm = gut */
#define LIMIT_CO2_MODERATE 1000  /**< 800-1000 ppm = mäßig */
#define LIMIT_CO2_BAD      1500  /**< 1000-1500 ppm = schlecht */
/** @} */

/** @name PM2.5-Grenzwerte (µg/m³, WHO 2021)
 *  @{
 */
#define LIMIT_PM25_GOOD     5    /**< WHO 2021: ≤5 µg/m³ = gut */
#define LIMIT_PM25_MODERATE 15   /**< WHO 2021: ≤15 µg/m³ = mäßig */
#define LIMIT_PM25_BAD      25   /**< WHO 2021: ≤25 µg/m³ = schlecht */
/** @} */

/** @name VOC-Index Grenzwerte
 *  @{
 */
#define LIMIT_VOC_GOOD     100   /**< ≤100 = gut */
#define LIMIT_VOC_MODERATE 200   /**< 100-200 = mäßig */
#define LIMIT_VOC_BAD      300   /**< 200-300 = schlecht */
/** @} */

/** @name Temperatur-Grenzwerte (°C, Behaglichkeit)
 *  @{
 */
#define LIMIT_TEMP_GOOD_MIN  18  /**< Untere Grenze Komfortbereich */
#define LIMIT_TEMP_GOOD_MAX  26  /**< Obere Grenze Komfortbereich */
#define LIMIT_TEMP_WARN_MIN  15  /**< Untere Grenze Akzeptabel */
#define LIMIT_TEMP_WARN_MAX  30  /**< Obere Grenze Akzeptabel */
/** @} */

/** @name Luftfeuchte-Grenzwerte (%, Behaglichkeit)
 *  @{
 */
#define LIMIT_HUM_GOOD_MIN  30   /**< Untere Grenze Komfortbereich */
#define LIMIT_HUM_GOOD_MAX  60   /**< Obere Grenze Komfortbereich */
#define LIMIT_HUM_WARN_MIN  20   /**< Untere Grenze Akzeptabel */
#define LIMIT_HUM_WARN_MAX  70   /**< Obere Grenze Akzeptabel */
/** @} */

// ============================================
// FARBKLASSIFIZIERUNG - SENSOREN
// ============================================

/**
 * @brief Bestimmt die Farbe basierend auf CO2-Pegel
 * @param co2 CO2-Konzentration in ppm
 * @return RGB565 Farbe (COLOR_GOOD/MODERATE/BAD/DANGER)
 */
inline uint16_t getCO2Color(int co2) {
  if (co2 < LIMIT_CO2_GOOD)     return COLOR_GOOD;
  if (co2 < LIMIT_CO2_MODERATE) return COLOR_MODERATE;
  if (co2 < LIMIT_CO2_BAD)      return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * @brief Bestimmt die Farbe basierend auf PM2.5-Pegel (WHO 2021)
 * @param pm25 Feinstaubmenge in µg/m³
 * @return RGB565 Farbe (COLOR_GOOD/MODERATE/BAD/DANGER)
 */
inline uint16_t getPM25Color(int pm25) {
  if (pm25 <= LIMIT_PM25_GOOD)     return COLOR_GOOD;
  if (pm25 <= LIMIT_PM25_MODERATE) return COLOR_MODERATE;
  if (pm25 <= LIMIT_PM25_BAD)      return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * @brief Bestimmt die Farbe basierend auf VOC-Index
 * @param voc VOC-Index (0-500)
 * @return RGB565 Farbe (COLOR_GOOD/MODERATE/BAD/DANGER)
 */
inline uint16_t getVOCColor(int voc) {
  if (voc <= LIMIT_VOC_GOOD)     return COLOR_GOOD;
  if (voc <= LIMIT_VOC_MODERATE) return COLOR_MODERATE;
  if (voc <= LIMIT_VOC_BAD)      return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * @brief Bestimmt die Farbe basierend auf Temperatur
 * @param temp Temperatur in °C
 * @return RGB565 Farbe (COLOR_GOOD/MODERATE/BAD)
 */
inline uint16_t getTempColor(float temp) {
  if (temp >= LIMIT_TEMP_GOOD_MIN && temp <= LIMIT_TEMP_GOOD_MAX) return COLOR_GOOD;
  if (temp >= LIMIT_TEMP_WARN_MIN && temp <= LIMIT_TEMP_WARN_MAX) return COLOR_MODERATE;
  return COLOR_BAD;
}

/**
 * @brief Bestimmt die Farbe basierend auf Luftfeuchte
 * @param hum Luftfeuchte in % rel.
 * @return RGB565 Farbe (COLOR_GOOD/MODERATE/BAD)
 */
inline uint16_t getHumColor(float hum) {
  if (hum >= LIMIT_HUM_GOOD_MIN && hum <= LIMIT_HUM_GOOD_MAX) return COLOR_GOOD;
  if (hum >= LIMIT_HUM_WARN_MIN && hum <= LIMIT_HUM_WARN_MAX) return COLOR_MODERATE;
  return COLOR_BAD;
}

#endif
