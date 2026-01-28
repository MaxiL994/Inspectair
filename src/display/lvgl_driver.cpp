/**
 * LVGL 9 Display-Treiber für LovyanGFX
 * InspectAir - ESP32-S3 mit ST7796S (480x320)
 */

#include "lvgl_driver.h"
#include "pins.h"
#include <Arduino.h>

// Globales Display-Objekt
static LGFX tft;

// LVGL Tick
static uint32_t last_tick = 0;

// LVGL Display und Buffer
static lv_display_t* display = nullptr;
static uint8_t* buf1 = nullptr;
static uint8_t* buf2 = nullptr;

/**
 * Display Flush Callback für LVGL 9
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
 * Initialisiert LVGL und den Display-Treiber
 */
void lvgl_init(void) {
    // Backlight Pin konfigurieren
    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, HIGH);
    
    // LovyanGFX Display initialisieren
    tft.init();
    tft.setRotation(1);  // Landscape Mode
    tft.fillScreen(TFT_BLACK);
    
    // LVGL initialisieren
    lv_init();
    
    // Buffer Größe in Bytes (16-bit = 2 bytes pro Pixel)
    size_t buf_size = LVGL_BUF_SIZE * 2;
    
    // PSRAM für Buffer nutzen
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
        Serial.println("[LVGL] FEHLER: Buffer-Allokation fehlgeschlagen!");
        return;
    }
    
    // Display erstellen (LVGL 9 API)
    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(display, lvgl_flush_cb);
    lv_display_set_buffers(display, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    Serial.println("[LVGL 9] Display initialisiert");
    Serial.printf("[LVGL 9] Auflösung: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
}

/**
 * LVGL Loop Handler - MUSS regelmäßig aufgerufen werden!
 */
void lvgl_loop(void) {
    // LVGL Tick aktualisieren (WICHTIG für LVGL 9!)
    uint32_t now = millis();
    uint32_t elapsed = now - last_tick;
    if (elapsed > 0) {
        lv_tick_inc(elapsed);
        last_tick = now;
    }
    
    // Timer/Tasks verarbeiten
    lv_timer_handler();
}

/**
 * Gibt das TFT-Objekt zurück
 */
LGFX* lvgl_get_tft(void) {
    return &tft;
}
