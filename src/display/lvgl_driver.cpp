/**
 * LVGL 9 Display Driver for LovyanGFX
 * InspectAir - ESP32-S3 with ST7796S (480x320)
 */

#include "lvgl_driver.h"
#include "pins.h"
#include <Arduino.h>

// Global display object
static LGFX tft;

// LVGL Tick
static uint32_t last_tick = 0;

// LVGL display and buffer
static lv_display_t* display = nullptr;
static uint8_t* buf1 = nullptr;
static uint8_t* buf2 = nullptr;

/**
 * Display flush callback for LVGL 9
 */
static void lvgl_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((uint16_t*)px_map, w * h);
    tft.endWrite();
    
    lv_display_flush_ready(disp);
}

/**
 * Initializes LVGL and the display driver
 */
void lvgl_init(void) {
    // Configure backlight pin
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);
    
    // Initialize LovyanGFX display
    tft.init();
    tft.setRotation(1);  // Landscape Mode
    tft.fillScreen(TFT_BLACK);
    
    // Initialize LVGL
    lv_init();
    
    // Buffer size in bytes (16-bit = 2 bytes per pixel)
    size_t buf_size = LVGL_BUF_SIZE * 2;
    
    // Use PSRAM for buffer
    #ifdef BOARD_HAS_PSRAM
        buf1 = (uint8_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
        buf2 = (uint8_t*)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
        if (!buf1) buf1 = (uint8_t*)malloc(buf_size);
        if (!buf2) buf2 = (uint8_t*)malloc(buf_size);
    #else
        buf1 = (uint8_t*)malloc(buf_size);
        buf2 = (uint8_t*)malloc(buf_size);
    #endif
    
    if (!buf1 || !buf2) {
        Serial.println("[LVGL] ERROR: Buffer allocation failed!");
        return;
    }
    
    // Create display (LVGL 9 API)
    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(display, lvgl_flush_cb);
    lv_display_set_buffers(display, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    Serial.println("[LVGL 9] Display initialized");
    Serial.printf("[LVGL 9] Resolution: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
}

/**
 * LVGL loop handler - MUST be called regularly!
 */
void lvgl_loop(void) {
    // Update LVGL tick (IMPORTANT for LVGL 9!)
    uint32_t now = millis();
    uint32_t elapsed = now - last_tick;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        last_tick = now;
    }
    
    // Process timers/tasks
    lv_timer_handler();
}

/**
 * Returns the TFT object
 */
LGFX* lvgl_get_tft(void) {
    return &tft;
}
