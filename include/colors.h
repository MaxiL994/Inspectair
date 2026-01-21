#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

// ============================================
// FARBDEFINITIONEN (RGB565)
// ============================================

// === PRIMÄRFARBEN ===
#define COLOR_GOOD       0x07E0  // Grün
#define COLOR_MODERATE   0xFFE0  // Gelb
#define COLOR_BAD        0xFD20  // Orange
#define COLOR_DANGER     0xF800  // Rot

// === HINTERGRUND & TEXT ===
#define COLOR_BG         0x0000  // Schwarz
#define COLOR_TEXT       0xFFFF  // Weiß
#define COLOR_LABEL      0xAD55  // Grau
#define COLOR_BOX_BG     0x1082  // Dunkles Blaugrau
#define COLOR_TITLE      0x07FF  // Cyan

// ============================================
// FARBKLASSIFIZIERUNG - SENSOREN
// ============================================

/**
 * Bestimmt die Farbe basierend auf CO2-Pegel
 * @param co2 CO2-Konzentration in ppm
 * @return RGB565 Farbe
 */
inline uint16_t getCO2Color(int co2) {
  if (co2 < 800)   return COLOR_GOOD;
  if (co2 < 1000)  return COLOR_MODERATE;
  if (co2 < 1500)  return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * Bestimmt die Farbe basierend auf PM2.5-Pegel
 * @param pm25 Feinstaubmenge in µg/m³
 * @return RGB565 Farbe
 */
inline uint16_t getPM25Color(int pm25) {
  if (pm25 < 12)   return COLOR_GOOD;
  if (pm25 < 35)   return COLOR_MODERATE;
  if (pm25 < 55)   return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * Bestimmt die Farbe basierend auf VOC-Index
 * @param voc VOC-Index (0-500)
 * @return RGB565 Farbe
 */
inline uint16_t getVOCColor(int voc) {
  if (voc < 100)   return COLOR_GOOD;
  if (voc < 200)   return COLOR_MODERATE;
  if (voc < 300)   return COLOR_BAD;
  return COLOR_DANGER;
}

/**
 * Bestimmt die Farbe basierend auf Temperatur
 * @param temp Temperatur in °C
 * @return RGB565 Farbe
 */
inline uint16_t getTempColor(float temp) {
  if (temp >= 18 && temp <= 24) return COLOR_GOOD;
  if (temp >= 15 && temp <= 28) return COLOR_MODERATE;
  return COLOR_BAD;
}

/**
 * Bestimmt die Farbe basierend auf Luftfeuchte
 * @param hum Luftfeuchte in % rel.
 * @return RGB565 Farbe
 */
inline uint16_t getHumColor(float hum) {
  if (hum >= 40 && hum <= 60) return COLOR_GOOD;
  if (hum >= 30 && hum <= 70) return COLOR_MODERATE;
  return COLOR_BAD;
}

#endif
