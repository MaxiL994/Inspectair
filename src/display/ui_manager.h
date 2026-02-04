/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LVGL 9 UI MANAGER
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Display: 480x320 (ST7796S)
 * Framework: LVGL 9.x
 * 
 * Multi-Screen UI System:
 * - Screen 1 (Overview): Large AQI right, 2 tiles Temp/Hum left
 * - Screen 2 (Detail): Small AQI, 4 tiles with all values
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include "sensor_types.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * FONT DECLARATIONS (Custom fonts with umlauts)
 * ═══════════════════════════════════════════════════════════════════════════ */
LV_FONT_DECLARE(ui_font_12);
LV_FONT_DECLARE(ui_font_16);
LV_FONT_DECLARE(ui_font_20);
LV_FONT_DECLARE(ui_font_28);
LV_FONT_DECLARE(ui_font_48);

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */
enum UIScreen {
    UI_SCREEN_TREE     = 0,  // Tree animation (start screen)
    UI_SCREEN_OVERVIEW = 1,  // Large AQI + 2 tiles (minimalist)
    UI_SCREEN_DETAIL   = 2,  // Small AQI + 4 tiles (full info)
    UI_SCREEN_ANALOG   = 3,  // Analog cockpit gauges
    UI_SCREEN_BUBBLE   = 4,  // Dynamic circles (modern bubbles)
    UI_SCREEN_COUNT    = 5   // Number of screens
};

/* ═══════════════════════════════════════════════════════════════════════════
 * API FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * Initializes all LVGL UI screens
 * Must be called after lvgl_init()
 */
void ui_init(void);

/**
 * Switches to the next screen (cyclic)
 */
void ui_nextScreen(void);

/**
 * Switches to a specific screen
 * @param screen The desired screen
 */
void ui_setScreen(UIScreen screen);

/**
 * Returns the current screen
 * @return The current screen
 */
UIScreen ui_getCurrentScreen(void);

/**
 * Updates the time display (on all screens)
 * @param hour Hour (0-23)
 * @param minute Minute (0-59)
 * @param second Second (0-59)
 */
void ui_updateTime(int hour, int minute, int second);

/**
 * Updates the date (on all screens)
 * @param date_str Formatted date string (e.g. "Di, 14. Jan")
 */
void ui_updateDate(const char* date_str);

/**
 * Updates all sensor values (on all screens)
 * @param temp Temperature in °C
 * @param hum Humidity in %
 * @param co2 CO2 in ppm
 * @param pm25 PM2.5 in µg/m³
 * @param voc VOC index (0-500)
 */
void ui_updateSensorValues(float temp, float hum, int co2, int pm25, int voc);

/**
 * Updates all sensor values from SensorReadings structure
 */
void ui_updateSensors(const SensorReadings& readings);

#endif // UI_MANAGER_H
