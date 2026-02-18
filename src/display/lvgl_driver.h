/**
 * LVGL 9 Display Driver for LovyanGFX
 * InspectAir - ESP32-S3 with ST7796S (480x320)
 */

#ifndef LVGL_DRIVER_H
#define LVGL_DRIVER_H

#include <lvgl.h>
#include "display_config.h"

// Display size
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

// Buffer size (1/10 of display)
#define LVGL_BUF_SIZE (SCREEN_WIDTH * 32)

/**
 * Initializes LVGL and the display driver
 * Must be called BEFORE all other LVGL calls
 */
void lvgl_init(void);

/**
 * Sets the display brightness (0-255)
 */
void lvgl_setBrightness(uint8_t brightness);

/**
 * Returns the current brightness
 */
uint8_t lvgl_getBrightness(void);

/**
 * Must be called regularly in loop
 * Processes LVGL timers and events
 */
void lvgl_loop(void);

/**
 * Returns the TFT object (for direct access)
 */
LGFX* lvgl_get_tft(void);

#endif // LVGL_DRIVER_H
