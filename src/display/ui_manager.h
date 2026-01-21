#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <stdint.h>
#include "../include/display_config.h"
#include "../include/sensor_types.h"

// ============================================
// UI MANAGER - DISPLAY RENDERING
// ============================================

extern LGFX tft;  // Global display object

/**
 * Initialisiert das Display und zeigt Startbildschirm
 */
void ui_init(void);

/**
 * Zeichnet eine Sensor-Box mit Wert und Einheit
 * @param x X-Position
 * @param y Y-Position
 * @param w Breite
 * @param h Höhe
 * @param label Beschriftung (z.B. "TEMPERATUR")
 * @param value Messwert als String
 * @param unit Einheit (z.B. "°C")
 * @param valueColor Farbe des Messwertes (RGB565)
 */
void ui_drawSensorBox(int x, int y, int w, int h, 
                      const char* label, const char* value, 
                      const char* unit, uint16_t valueColor);

/**
 * Updated alle Sensor-Boxen mit aktuellen Werten
 * @param readings Aktuelle Sensormesswerte
 */
void ui_updateDisplay(const SensorReadings& readings);

/**
 * Zeigt Fehlermeldung auf dem Display
 * @param message Fehlermeldung
 */
void ui_showError(const char* message);

/**
 * Setzt Display-Helligkeit
 * @param brightness 0-255
 */
void ui_setBacklight(uint8_t brightness);

#endif
