/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LVGL 9 UI MANAGER
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Display: 480x320 (ST7796S)
 * Framework: LVGL 9.x
 * 
 * Multi-Screen UI System:
 * - Screen 1 (Übersicht): Große AQI rechts, 2 Kacheln Temp/Hum links
 * - Screen 2 (Detail): Kleine AQI, 4 Kacheln mit allen Werten
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
 * SCREEN TYPEN
 * ═══════════════════════════════════════════════════════════════════════════ */
enum UIScreen {
    UI_SCREEN_TREE     = 0,  // Baum-Animation (Startbildschirm)
    UI_SCREEN_OVERVIEW = 1,  // Große AQI + 2 Kacheln (minimalistisch)
    UI_SCREEN_DETAIL   = 2,  // Kleine AQI + 4 Kacheln (volle Infos)
    UI_SCREEN_ANALOG   = 3,  // Analoge Cockpit-Instrumente
    UI_SCREEN_BUBBLE   = 4,  // Dynamische Kreise (Modern Bubbles)
    UI_SCREEN_COUNT    = 5   // Anzahl der Screens
};

/* ═══════════════════════════════════════════════════════════════════════════
 * API FUNKTIONEN
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * Initialisiert alle LVGL UI Screens
 * Muss nach lvgl_init() aufgerufen werden
 */
void ui_init(void);

/**
 * Wechselt zum nächsten Screen (zyklisch)
 */
void ui_nextScreen(void);

/**
 * Wechselt zu einem bestimmten Screen
 * @param screen Der gewünschte Screen
 */
void ui_setScreen(UIScreen screen);

/**
 * Gibt den aktuellen Screen zurück
 * @return Der aktuelle Screen
 */
UIScreen ui_getCurrentScreen(void);

/**
 * Aktualisiert die Uhrzeit-Anzeige (auf allen Screens)
 * @param hour Stunde (0-23)
 * @param minute Minute (0-59)
 * @param second Sekunde (0-59)
 */
void ui_updateTime(int hour, int minute, int second);

/**
 * Aktualisiert das Datum (auf allen Screens)
 * @param date_str Formatierter Datums-String (z.B. "Di, 14. Jan")
 */
void ui_updateDate(const char* date_str);

/**
 * Aktualisiert alle Sensorwerte (auf allen Screens)
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
