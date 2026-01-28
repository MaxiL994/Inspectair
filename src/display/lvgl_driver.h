/**
 * LVGL 9 Display-Treiber für LovyanGFX
 * InspectAir - ESP32-S3 mit ST7796S (480x320)
 */

#ifndef LVGL_DRIVER_H
#define LVGL_DRIVER_H

#include <lvgl.h>
#include "display_config.h"

// Display-Größe
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

// Buffer-Größe (1/10 des Displays)
#define LVGL_BUF_SIZE (SCREEN_WIDTH * 32)

/**
 * Initialisiert LVGL und den Display-Treiber
 * Muss VOR allen anderen LVGL-Aufrufen ausgeführt werden
 */
void lvgl_init(void);

/**
 * Muss regelmäßig im Loop aufgerufen werden
 * Verarbeitet LVGL Timer und Events
 */
void lvgl_loop(void);

/**
 * Gibt das TFT-Objekt zurück (für direkte Zugriffe)
 */
LGFX* lvgl_get_tft(void);

#endif // LVGL_DRIVER_H
