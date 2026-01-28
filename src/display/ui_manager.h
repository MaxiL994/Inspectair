/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LVGL 9 UI MANAGER
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Display: 480x320 (ST7796S)
 * Framework: LVGL 9.x
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "sensor_types.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * FONT DEKLARATIONEN (Custom Fonts mit Umlauten)
 * ═══════════════════════════════════════════════════════════════════════════ */
LV_FONT_DECLARE(ui_font_12);
LV_FONT_DECLARE(ui_font_16);
LV_FONT_DECLARE(ui_font_20);
LV_FONT_DECLARE(ui_font_28);
LV_FONT_DECLARE(ui_font_48);

/* ═══════════════════════════════════════════════════════════════════════════
 * API FUNKTIONEN
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * Initialisiert die LVGL UI
 * Muss nach lvgl_init() aufgerufen werden
 */
void ui_init(void);

/**
 * Aktualisiert die Uhrzeit-Anzeige
 * @param hour Stunde (0-23)
 * @param minute Minute (0-59)
 */
void ui_updateTime(int hour, int minute, int second);

/**
 * Aktualisiert das Datum
 * @param date_str Formatierter Datums-String (z.B. "Di, 14. Jan")
 */
void ui_updateDate(const char* date_str);

/**
 * Aktualisiert alle Sensorwerte
 * @param temp Temperatur in °C
 * @param hum Luftfeuchtigkeit in %
 * @param co2 CO2 in ppm
 * @param pm25 PM2.5 in µg/m³
 * @param voc VOC Index (0-500)
 */
void ui_updateSensorValues(float temp, float hum, int co2, int pm25, int voc);

/**
 * Aktualisiert alle Sensorwerte aus SensorReadings-Struktur
 */
void ui_updateSensors(const SensorReadings& readings);

#endif // UI_MANAGER_H
