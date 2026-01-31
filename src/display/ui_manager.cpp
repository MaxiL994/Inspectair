/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LVGL 9 UI MANAGER (Multi-Screen)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Display: 480x320 (ST7796S)
 * Framework: LVGL 9.x
 * 
 * Screen 1 (Übersicht): Große AQI rechts (volle Höhe), 2 große Kacheln links
 * Screen 2 (Detail):    Kleine AQI oben, 4 Kacheln unten
 */

#include "ui_manager.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * EMOJI BILDER (LVGL 9 kompatibel)
 * ═══════════════════════════════════════════════════════════════════════════ */
LV_IMAGE_DECLARE(emoji_happy);
LV_IMAGE_DECLARE(emoji_meh);
LV_IMAGE_DECLARE(emoji_mask);

// Sensor-Icons (14x14)
LV_IMAGE_DECLARE(emoji_icon_thermometer);
LV_IMAGE_DECLARE(emoji_icon_droplet);
LV_IMAGE_DECLARE(emoji_icon_cloud);
LV_IMAGE_DECLARE(emoji_icon_dash);
LV_IMAGE_DECLARE(emoji_icon_nose);

/* ═══════════════════════════════════════════════════════════════════════════
 * FONT KONFIGURATION
 * ═══════════════════════════════════════════════════════════════════════════ */
#define FONT_12  &ui_font_12
#define FONT_16  &ui_font_16
#define FONT_20  &ui_font_20
#define FONT_28  &ui_font_28
#define FONT_48  &ui_font_48

// Texte mit Umlauten
#define TXT_LUFTQUALITAET   "Luftqualität"
#define TXT_SEHR_GUT        "Sehr gut"
#define TXT_MAESSIG         "Mäßig"
#define TXT_SCHLECHT        "Schlecht"
#define TXT_TEMPERATUR      "Temperatur"
#define TXT_FEUCHTE         "Feuchte"
#define TXT_UNIT_TEMP       "°C"
#define TXT_UNIT_PM         "ug/m3"

/* ═══════════════════════════════════════════════════════════════════════════
 * FARBEN
 * ═══════════════════════════════════════════════════════════════════════════ */
#define COLOR_BG        lv_color_hex(0xFAFAFA)
#define COLOR_CARD      lv_color_hex(0xFFFFFF)
#define COLOR_TEXT      lv_color_hex(0x1A1A1A)
#define COLOR_TEXT_L    lv_color_hex(0x666666)
#define COLOR_DATE      lv_color_hex(0x888888)

#define COLOR_GOOD      lv_color_hex(0x10B981)
#define COLOR_WARN      lv_color_hex(0xF59E0B)
#define COLOR_BAD       lv_color_hex(0xEF4444)

#define COLOR_BAR_BG    lv_color_hex(0xF0F0F0)
#define COLOR_RING_BG   lv_color_hex(0xEEEEEE)

/* ═══════════════════════════════════════════════════════════════════════════
 * STATUS HELPER (shared)
 * ═══════════════════════════════════════════════════════════════════════════ */
enum Status { GOOD = 0, WARN = 1, BAD = 2 };

static Status get_temp_status(float t) {
    if (t >= 18 && t <= 26) return GOOD;
    if (t >= 15 && t <= 30) return WARN;
    return BAD;
}

static Status get_hum_status(float h) {
    if (h >= 30 && h <= 60) return GOOD;
    if (h >= 20 && h <= 70) return WARN;
    return BAD;
}

static Status get_co2_status(int c) {
    if (c <= 1000) return GOOD;
    if (c <= 1500) return WARN;
    return BAD;
}

static Status get_pm25_status(int p) {
    if (p <= 15) return GOOD;
    if (p <= 25) return WARN;
    return BAD;
}

static Status get_voc_status(int voc) {
    if (voc <= 150) return GOOD;
    if (voc <= 250) return WARN;
    return BAD;
}

static Status get_air_quality(int co2, int pm25) {
    Status s1 = get_co2_status(co2);
    Status s2 = get_pm25_status(pm25);
    if (s1 == BAD || s2 == BAD) return BAD;
    if (s1 == WARN || s2 == WARN) return WARN;
    return GOOD;
}

static lv_color_t get_status_color(Status s) {
    if (s == GOOD) return COLOR_GOOD;
    if (s == WARN) return COLOR_WARN;
    return COLOR_BAD;
}

static const char* get_status_text(Status s) {
    if (s == GOOD) return TXT_SEHR_GUT;
    if (s == WARN) return TXT_MAESSIG;
    return TXT_SCHLECHT;
}

static const lv_image_dsc_t* get_status_emoji_img(Status s) {
    if (s == GOOD) return &emoji_happy;
    if (s == WARN) return &emoji_meh;
    return &emoji_mask;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SHARED STYLES
 * ═══════════════════════════════════════════════════════════════════════════ */
static lv_style_t style_card;
static lv_style_t style_bar_bg;
static lv_style_t style_bar_good;
static lv_style_t style_bar_warn;
static lv_style_t style_bar_bad;
static bool styles_initialized = false;

static lv_style_t* get_bar_style(Status s) {
    if (s == GOOD) return &style_bar_good;
    if (s == WARN) return &style_bar_warn;
    return &style_bar_bad;
}

static void init_styles() {
    if (styles_initialized) return;
    styles_initialized = true;

    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_CARD);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_border_width(&style_card, 0);
    lv_style_set_shadow_width(&style_card, 12);
    lv_style_set_shadow_color(&style_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_card, LV_OPA_20);
    lv_style_set_shadow_offset_y(&style_card, 4);
    lv_style_set_pad_all(&style_card, 12);

    lv_style_init(&style_bar_bg);
    lv_style_set_bg_color(&style_bar_bg, COLOR_BAR_BG);
    lv_style_set_bg_opa(&style_bar_bg, LV_OPA_COVER);
    lv_style_set_radius(&style_bar_bg, 2);

    lv_style_init(&style_bar_good);
    lv_style_set_bg_color(&style_bar_good, COLOR_GOOD);
    lv_style_set_bg_opa(&style_bar_good, LV_OPA_COVER);
    lv_style_set_radius(&style_bar_good, 2);

    lv_style_init(&style_bar_warn);
    lv_style_set_bg_color(&style_bar_warn, COLOR_WARN);
    lv_style_set_bg_opa(&style_bar_warn, LV_OPA_COVER);
    lv_style_set_radius(&style_bar_warn, 2);

    lv_style_init(&style_bar_bad);
    lv_style_set_bg_color(&style_bar_bad, COLOR_BAD);
    lv_style_set_bg_opa(&style_bar_bad, LV_OPA_COVER);
    lv_style_set_radius(&style_bar_bad, 2);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */
static lv_obj_t* screens[UI_SCREEN_COUNT] = {nullptr, nullptr};
static UIScreen current_screen = UI_SCREEN_OVERVIEW;

// Gecachte Sensorwerte für Screen-Updates
static float cached_temp = 0;
static float cached_hum = 0;
static int cached_co2 = 0;
static int cached_pm25 = 0;
static int cached_voc = 0;
static int cached_hour = 0;
static int cached_min = 0;
static int cached_sec = 0;
static char cached_date[20] = "Di, 28. Jan";

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN 1: ÜBERSICHT (Große AQI + 2 große Kacheln)
 * ═══════════════════════════════════════════════════════════════════════════
 * Layout:
 * ┌──────────────────────────────────────────────────────┐
 * │  Uhr + Datum (oben links)    │   Große AQI Box      │
 * │  12:34 56                    │   (volle Höhe)       │
 * │  Di, 28. Jan                 │                      │
 * ├──────────────────────────────┤   🙂 Luftqualität   │
 * │    TEMPERATUR    │  FEUCHTE  │      Sehr gut        │
 * │    🌡️  23.5°C    │  💧 45%  │                      │
 * │    ═══════       │  ═══════  │   Arc + Emoji        │
 * └──────────────────────────────┴──────────────────────┘
 */

// Screen 1 UI Elemente
static lv_obj_t* s1_lbl_time = nullptr;
static lv_obj_t* s1_lbl_seconds = nullptr;
static lv_obj_t* s1_lbl_date = nullptr;
static lv_obj_t* s1_aqi_box = nullptr;
static lv_obj_t* s1_arc_aqi = nullptr;
static lv_obj_t* s1_img_emoji = nullptr;
static lv_obj_t* s1_lbl_aqi_title = nullptr;
static lv_obj_t* s1_lbl_aqi_status = nullptr;

// 2 große Kacheln
static lv_obj_t* s1_card_temp = nullptr;
static lv_obj_t* s1_lbl_temp_title = nullptr;
static lv_obj_t* s1_img_temp = nullptr;
static lv_obj_t* s1_lbl_temp_value = nullptr;
static lv_obj_t* s1_lbl_temp_unit = nullptr;
static lv_obj_t* s1_bar_temp = nullptr;

static lv_obj_t* s1_card_hum = nullptr;
static lv_obj_t* s1_lbl_hum_title = nullptr;
static lv_obj_t* s1_img_hum = nullptr;
static lv_obj_t* s1_lbl_hum_value = nullptr;
static lv_obj_t* s1_lbl_hum_unit = nullptr;
static lv_obj_t* s1_bar_hum = nullptr;

static void create_screen1() {
    screens[UI_SCREEN_OVERVIEW] = lv_obj_create(NULL);
    lv_obj_t* scr = screens[UI_SCREEN_OVERVIEW];
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Layout-Konstanten für Screen 1
    const int AQI_BOX_W = 180;
    const int AQI_BOX_H = 280;
    const int AQI_BOX_X = 480 - AQI_BOX_W - 10;
    const int AQI_BOX_Y = 20;
    
    const int CARD_W = 130;
    const int CARD_H = 130;
    const int CARDS_Y = 160;

    // ─────────────────────────────────────────────────────────────────────
    // UHRZEIT (oben links) - gleiche Position wie Screen 2
    // ─────────────────────────────────────────────────────────────────────
    s1_lbl_time = lv_label_create(scr);
    lv_obj_set_style_text_font(s1_lbl_time, FONT_48, 0);
    lv_obj_set_style_text_color(s1_lbl_time, COLOR_TEXT, 0);
    lv_label_set_text(s1_lbl_time, "00:00");
    lv_obj_set_pos(s1_lbl_time, 65, 55);

    s1_lbl_seconds = lv_label_create(scr);
    lv_obj_set_style_text_font(s1_lbl_seconds, FONT_28, 0);
    lv_obj_set_style_text_color(s1_lbl_seconds, COLOR_TEXT_L, 0);
    lv_label_set_text(s1_lbl_seconds, "00");
    lv_obj_set_pos(s1_lbl_seconds, 189, 65);

    s1_lbl_date = lv_label_create(scr);
    lv_obj_set_style_text_font(s1_lbl_date, FONT_16, 0);
    lv_obj_set_style_text_color(s1_lbl_date, COLOR_DATE, 0);
    lv_label_set_text(s1_lbl_date, "Di, 28. Jan");
    lv_obj_set_pos(s1_lbl_date, 75, 115);

    // ─────────────────────────────────────────────────────────────────────
    // GROSSE AQI BOX (rechts, volle Höhe)
    // ─────────────────────────────────────────────────────────────────────
    s1_aqi_box = lv_obj_create(scr);
    lv_obj_add_style(s1_aqi_box, &style_card, 0);
    lv_obj_set_size(s1_aqi_box, AQI_BOX_W, AQI_BOX_H);
    lv_obj_set_pos(s1_aqi_box, AQI_BOX_X, AQI_BOX_Y);
    lv_obj_remove_flag(s1_aqi_box, LV_OBJ_FLAG_SCROLLABLE);

    // Titel oben
    s1_lbl_aqi_title = lv_label_create(s1_aqi_box);
    lv_obj_set_style_text_font(s1_lbl_aqi_title, FONT_16, 0);
    lv_obj_set_style_text_color(s1_lbl_aqi_title, COLOR_TEXT_L, 0);
    lv_label_set_text(s1_lbl_aqi_title, TXT_LUFTQUALITAET);
    lv_obj_align(s1_lbl_aqi_title, LV_ALIGN_TOP_MID, 0, 5);

    // Großer AQI Ring (mittig)
    s1_arc_aqi = lv_arc_create(s1_aqi_box);
    lv_obj_set_size(s1_arc_aqi, 120, 120);
    lv_obj_align(s1_arc_aqi, LV_ALIGN_CENTER, 0, -10);
    lv_arc_set_rotation(s1_arc_aqi, 270);
    lv_arc_set_bg_angles(s1_arc_aqi, 0, 360);
    lv_arc_set_range(s1_arc_aqi, 0, 100);
    lv_arc_set_value(s1_arc_aqi, 100);
    lv_obj_remove_style(s1_arc_aqi, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(s1_arc_aqi, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_width(s1_arc_aqi, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s1_arc_aqi, COLOR_RING_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s1_arc_aqi, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s1_arc_aqi, COLOR_GOOD, LV_PART_INDICATOR);

    // Großes Emoji im Ring
    s1_img_emoji = lv_image_create(s1_arc_aqi);
    lv_image_set_src(s1_img_emoji, &emoji_happy);
    lv_image_set_scale(s1_img_emoji, 384);  // 1.5x größer
    lv_obj_center(s1_img_emoji);

    // Status Text unten
    s1_lbl_aqi_status = lv_label_create(s1_aqi_box);
    lv_obj_set_style_text_font(s1_lbl_aqi_status, FONT_20, 0);
    lv_obj_set_style_text_color(s1_lbl_aqi_status, COLOR_GOOD, 0);
    lv_label_set_text(s1_lbl_aqi_status, TXT_SEHR_GUT);
    lv_obj_align(s1_lbl_aqi_status, LV_ALIGN_BOTTOM_MID, 0, -10);

    // ─────────────────────────────────────────────────────────────────────
    // GROSSE TEMPERATUR-KACHEL (links unten)
    // ─────────────────────────────────────────────────────────────────────
    s1_card_temp = lv_obj_create(scr);
    lv_obj_add_style(s1_card_temp, &style_card, 0);
    lv_obj_set_size(s1_card_temp, CARD_W, CARD_H);
    lv_obj_set_pos(s1_card_temp, 10, CARDS_Y);
    lv_obj_remove_flag(s1_card_temp, LV_OBJ_FLAG_SCROLLABLE);

    s1_lbl_temp_title = lv_label_create(s1_card_temp);
    lv_obj_set_style_text_font(s1_lbl_temp_title, FONT_16, 0);
    lv_obj_set_style_text_color(s1_lbl_temp_title, COLOR_TEXT_L, 0);
    lv_label_set_text(s1_lbl_temp_title, "Temp");
    lv_obj_set_pos(s1_lbl_temp_title, 0, 0);

    s1_img_temp = lv_image_create(s1_card_temp);
    lv_image_set_src(s1_img_temp, &emoji_icon_thermometer);
    lv_image_set_scale(s1_img_temp, 384);  // 1.5x
    lv_obj_align(s1_img_temp, LV_ALIGN_TOP_RIGHT, -5, 0);

    s1_lbl_temp_value = lv_label_create(s1_card_temp);
    lv_obj_set_style_text_font(s1_lbl_temp_value, FONT_28, 0);  // FONT_28 hat Komma und °
    lv_obj_set_style_text_color(s1_lbl_temp_value, COLOR_TEXT, 0);
    lv_label_set_text(s1_lbl_temp_value, "--");
    lv_obj_set_pos(s1_lbl_temp_value, 0, 40);

    s1_lbl_temp_unit = lv_label_create(s1_card_temp);
    lv_obj_set_style_text_font(s1_lbl_temp_unit, FONT_16, 0);
    lv_obj_set_style_text_color(s1_lbl_temp_unit, COLOR_TEXT_L, 0);
    lv_label_set_text(s1_lbl_temp_unit, TXT_UNIT_TEMP);
    lv_obj_set_pos(s1_lbl_temp_unit, 55, 48);

    s1_bar_temp = lv_bar_create(s1_card_temp);
    lv_obj_set_size(s1_bar_temp, CARD_W - 24, 6);
    lv_obj_align(s1_bar_temp, LV_ALIGN_BOTTOM_LEFT, 0, -5);
    lv_bar_set_range(s1_bar_temp, 0, 100);
    lv_bar_set_value(s1_bar_temp, 33, LV_ANIM_OFF);
    lv_obj_add_style(s1_bar_temp, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(s1_bar_temp, &style_bar_good, LV_PART_INDICATOR);

    // ─────────────────────────────────────────────────────────────────────
    // GROSSE FEUCHTE-KACHEL (Mitte unten)
    // ─────────────────────────────────────────────────────────────────────
    s1_card_hum = lv_obj_create(scr);
    lv_obj_add_style(s1_card_hum, &style_card, 0);
    lv_obj_set_size(s1_card_hum, CARD_W, CARD_H);
    lv_obj_set_pos(s1_card_hum, 10 + CARD_W + 10, CARDS_Y);
    lv_obj_remove_flag(s1_card_hum, LV_OBJ_FLAG_SCROLLABLE);

    s1_lbl_hum_title = lv_label_create(s1_card_hum);
    lv_obj_set_style_text_font(s1_lbl_hum_title, FONT_16, 0);
    lv_obj_set_style_text_color(s1_lbl_hum_title, COLOR_TEXT_L, 0);
    lv_label_set_text(s1_lbl_hum_title, TXT_FEUCHTE);
    lv_obj_set_pos(s1_lbl_hum_title, 0, 0);

    s1_img_hum = lv_image_create(s1_card_hum);
    lv_image_set_src(s1_img_hum, &emoji_icon_droplet);
    lv_image_set_scale(s1_img_hum, 384);
    lv_obj_align(s1_img_hum, LV_ALIGN_TOP_RIGHT, -5, 0);

    s1_lbl_hum_value = lv_label_create(s1_card_hum);
    lv_obj_set_style_text_font(s1_lbl_hum_value, FONT_28, 0);  // FONT_28 für Konsistenz
    lv_obj_set_style_text_color(s1_lbl_hum_value, COLOR_TEXT, 0);
    lv_label_set_text(s1_lbl_hum_value, "--");
    lv_obj_set_pos(s1_lbl_hum_value, 0, 40);

    s1_lbl_hum_unit = lv_label_create(s1_card_hum);
    lv_obj_set_style_text_font(s1_lbl_hum_unit, FONT_16, 0);
    lv_obj_set_style_text_color(s1_lbl_hum_unit, COLOR_TEXT_L, 0);
    lv_label_set_text(s1_lbl_hum_unit, "%");
    lv_obj_set_pos(s1_lbl_hum_unit, 40, 48);

    s1_bar_hum = lv_bar_create(s1_card_hum);
    lv_obj_set_size(s1_bar_hum, CARD_W - 24, 6);
    lv_obj_align(s1_bar_hum, LV_ALIGN_BOTTOM_LEFT, 0, -5);
    lv_bar_set_range(s1_bar_hum, 0, 100);
    lv_bar_set_value(s1_bar_hum, 33, LV_ANIM_OFF);
    lv_obj_add_style(s1_bar_hum, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(s1_bar_hum, &style_bar_good, LV_PART_INDICATOR);

    Serial.println("[UI] Screen 1 (Übersicht) erstellt");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN 2: DETAIL (Kleine AQI + 4 Kacheln)
 * ═══════════════════════════════════════════════════════════════════════════
 * Layout:
 * ┌────────────────────────────────────────────────────────┐
 * │  Uhr + Datum         │  Kleine AQI Box                │
 * │  12:34 56            │  🙂 Luftqualität Sehr gut      │
 * │  Di, 28. Jan         │                                 │
 * ├──────────┬───────────┼─────────────┬──────────────────┤
 * │ Temp/Hum │    CO2    │  Feinstaub  │       VOC        │
 * │ combined │           │             │                  │
 * └──────────┴───────────┴─────────────┴──────────────────┘
 */

// Screen 2 UI Elemente
static lv_obj_t* s2_lbl_time = nullptr;
static lv_obj_t* s2_lbl_seconds = nullptr;
static lv_obj_t* s2_lbl_date = nullptr;
static lv_obj_t* s2_arc_aqi = nullptr;
static lv_obj_t* s2_img_emoji = nullptr;
static lv_obj_t* s2_lbl_aqi_title = nullptr;
static lv_obj_t* s2_lbl_aqi_status = nullptr;

// Kombinierte Klima-Kachel
static lv_obj_t* s2_card_climate = nullptr;
static lv_obj_t* s2_lbl_temp_title = nullptr;
static lv_obj_t* s2_img_temp = nullptr;
static lv_obj_t* s2_lbl_temp_value = nullptr;
static lv_obj_t* s2_lbl_temp_unit = nullptr;
static lv_obj_t* s2_bar_temp = nullptr;
static lv_obj_t* s2_lbl_hum_title = nullptr;
static lv_obj_t* s2_img_hum = nullptr;
static lv_obj_t* s2_lbl_hum_value = nullptr;
static lv_obj_t* s2_lbl_hum_unit = nullptr;
static lv_obj_t* s2_bar_hum = nullptr;

// Einzelne Kacheln
struct SensorCard {
    lv_obj_t* container;
    lv_obj_t* label;
    lv_obj_t* emoji;
    lv_obj_t* value;
    lv_obj_t* unit;
    lv_obj_t* bar;
    int width;
};
static SensorCard s2_cards[3];  // CO2, PM2.5, VOC

static void create_screen2() {
    screens[UI_SCREEN_DETAIL] = lv_obj_create(NULL);
    lv_obj_t* scr = screens[UI_SCREEN_DETAIL];
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Layout-Konstanten (wie bisherige UI)
    const int AQI_BOX_X = 270;
    const int AQI_BOX_Y = 38;
    const int AQI_BOX_W = 190;
    const int AQI_BOX_H = 90;
    const int CARD_Y = 178;
    const int CARD_H = 100;
    const int CARD_GAP = 5;
    const int CARD_START_X = 7;
    const int CLIMATE_CARD_W = 110;
    const int CLIMATE_CARD_H = 145;

    // ─────────────────────────────────────────────────────────────────────
    // UHRZEIT
    // ─────────────────────────────────────────────────────────────────────
    s2_lbl_time = lv_label_create(scr);
    lv_obj_set_style_text_font(s2_lbl_time, FONT_48, 0);
    lv_obj_set_style_text_color(s2_lbl_time, COLOR_TEXT, 0);
    lv_label_set_text(s2_lbl_time, "00:00");
    lv_obj_set_pos(s2_lbl_time, 65, 55);

    s2_lbl_seconds = lv_label_create(scr);
    lv_obj_set_style_text_font(s2_lbl_seconds, FONT_28, 0);
    lv_obj_set_style_text_color(s2_lbl_seconds, COLOR_TEXT_L, 0);
    lv_label_set_text(s2_lbl_seconds, "00");
    lv_obj_set_pos(s2_lbl_seconds, 189, 65);

    s2_lbl_date = lv_label_create(scr);
    lv_obj_set_style_text_font(s2_lbl_date, FONT_16, 0);
    lv_obj_set_style_text_color(s2_lbl_date, COLOR_DATE, 0);
    lv_label_set_text(s2_lbl_date, "Di, 28. Jan");
    lv_obj_set_pos(s2_lbl_date, 75, 115);

    // ─────────────────────────────────────────────────────────────────────
    // KLEINE AQI BOX
    // ─────────────────────────────────────────────────────────────────────
    lv_obj_t* aqi_box = lv_obj_create(scr);
    lv_obj_add_style(aqi_box, &style_card, 0);
    lv_obj_set_size(aqi_box, AQI_BOX_W, AQI_BOX_H);
    lv_obj_set_pos(aqi_box, AQI_BOX_X, AQI_BOX_Y);
    lv_obj_remove_flag(aqi_box, LV_OBJ_FLAG_SCROLLABLE);

    s2_arc_aqi = lv_arc_create(aqi_box);
    lv_obj_set_size(s2_arc_aqi, 60, 60);
    lv_obj_align(s2_arc_aqi, LV_ALIGN_LEFT_MID, 5, 0);
    lv_arc_set_rotation(s2_arc_aqi, 270);
    lv_arc_set_bg_angles(s2_arc_aqi, 0, 360);
    lv_arc_set_range(s2_arc_aqi, 0, 100);
    lv_arc_set_value(s2_arc_aqi, 100);
    lv_obj_remove_style(s2_arc_aqi, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(s2_arc_aqi, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_width(s2_arc_aqi, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s2_arc_aqi, COLOR_RING_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s2_arc_aqi, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s2_arc_aqi, COLOR_GOOD, LV_PART_INDICATOR);

    s2_img_emoji = lv_image_create(s2_arc_aqi);
    lv_image_set_src(s2_img_emoji, &emoji_happy);
    lv_obj_center(s2_img_emoji);

    s2_lbl_aqi_title = lv_label_create(aqi_box);
    lv_obj_set_style_text_font(s2_lbl_aqi_title, FONT_12, 0);
    lv_obj_set_style_text_color(s2_lbl_aqi_title, COLOR_TEXT_L, 0);
    lv_label_set_text(s2_lbl_aqi_title, TXT_LUFTQUALITAET);
    lv_obj_align(s2_lbl_aqi_title, LV_ALIGN_TOP_RIGHT, -5, 5);

    s2_lbl_aqi_status = lv_label_create(aqi_box);
    lv_obj_set_style_text_font(s2_lbl_aqi_status, FONT_16, 0);
    lv_obj_set_style_text_color(s2_lbl_aqi_status, COLOR_GOOD, 0);
    lv_label_set_text(s2_lbl_aqi_status, TXT_SEHR_GUT);
    lv_obj_align(s2_lbl_aqi_status, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    // ─────────────────────────────────────────────────────────────────────
    // KOMBINIERTE TEMP/FEUCHTE KACHEL
    // ─────────────────────────────────────────────────────────────────────
    s2_card_climate = lv_obj_create(scr);
    lv_obj_add_style(s2_card_climate, &style_card, 0);
    lv_obj_set_size(s2_card_climate, CLIMATE_CARD_W, CLIMATE_CARD_H);
    lv_obj_set_pos(s2_card_climate, CARD_START_X, 155);
    lv_obj_remove_flag(s2_card_climate, LV_OBJ_FLAG_SCROLLABLE);

    // Temperatur (oben)
    s2_lbl_temp_title = lv_label_create(s2_card_climate);
    lv_obj_set_style_text_font(s2_lbl_temp_title, FONT_12, 0);
    lv_obj_set_style_text_color(s2_lbl_temp_title, COLOR_TEXT_L, 0);
    lv_label_set_text(s2_lbl_temp_title, "Temp");
    lv_obj_set_pos(s2_lbl_temp_title, 0, 0);

    s2_img_temp = lv_image_create(s2_card_climate);
    lv_image_set_src(s2_img_temp, &emoji_icon_thermometer);
    lv_obj_set_pos(s2_img_temp, CLIMATE_CARD_W - 38, 0);

    s2_lbl_temp_value = lv_label_create(s2_card_climate);
    lv_obj_set_style_text_font(s2_lbl_temp_value, FONT_28, 0);
    lv_obj_set_style_text_color(s2_lbl_temp_value, COLOR_TEXT, 0);
    lv_label_set_text(s2_lbl_temp_value, "--");
    lv_obj_set_pos(s2_lbl_temp_value, 0, 18);

    s2_lbl_temp_unit = lv_label_create(s2_card_climate);
    lv_obj_set_style_text_font(s2_lbl_temp_unit, FONT_12, 0);
    lv_obj_set_style_text_color(s2_lbl_temp_unit, COLOR_TEXT_L, 0);
    lv_label_set_text(s2_lbl_temp_unit, TXT_UNIT_TEMP);
    lv_obj_set_pos(s2_lbl_temp_unit, 50, 34);

    s2_bar_temp = lv_bar_create(s2_card_climate);
    lv_obj_set_size(s2_bar_temp, CLIMATE_CARD_W - 24, 4);
    lv_obj_set_pos(s2_bar_temp, 0, 52);
    lv_bar_set_range(s2_bar_temp, 0, 100);
    lv_bar_set_value(s2_bar_temp, 33, LV_ANIM_OFF);
    lv_obj_add_style(s2_bar_temp, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(s2_bar_temp, &style_bar_good, LV_PART_INDICATOR);

    // Feuchte (unten)
    s2_lbl_hum_title = lv_label_create(s2_card_climate);
    lv_obj_set_style_text_font(s2_lbl_hum_title, FONT_12, 0);
    lv_obj_set_style_text_color(s2_lbl_hum_title, COLOR_TEXT_L, 0);
    lv_label_set_text(s2_lbl_hum_title, TXT_FEUCHTE);
    lv_obj_set_pos(s2_lbl_hum_title, 0, 62);

    s2_img_hum = lv_image_create(s2_card_climate);
    lv_image_set_src(s2_img_hum, &emoji_icon_droplet);
    lv_obj_set_pos(s2_img_hum, CLIMATE_CARD_W - 38, 62);

    s2_lbl_hum_value = lv_label_create(s2_card_climate);
    lv_obj_set_style_text_font(s2_lbl_hum_value, FONT_28, 0);
    lv_obj_set_style_text_color(s2_lbl_hum_value, COLOR_TEXT, 0);
    lv_label_set_text(s2_lbl_hum_value, "--");
    lv_obj_set_pos(s2_lbl_hum_value, 0, 80);

    s2_lbl_hum_unit = lv_label_create(s2_card_climate);
    lv_obj_set_style_text_font(s2_lbl_hum_unit, FONT_12, 0);
    lv_obj_set_style_text_color(s2_lbl_hum_unit, COLOR_TEXT_L, 0);
    lv_label_set_text(s2_lbl_hum_unit, "%");
    lv_obj_set_pos(s2_lbl_hum_unit, 35, 96);

    s2_bar_hum = lv_bar_create(s2_card_climate);
    lv_obj_set_size(s2_bar_hum, CLIMATE_CARD_W - 24, 4);
    lv_obj_set_pos(s2_bar_hum, 0, 114);
    lv_bar_set_range(s2_bar_hum, 0, 100);
    lv_bar_set_value(s2_bar_hum, 33, LV_ANIM_OFF);
    lv_obj_add_style(s2_bar_hum, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(s2_bar_hum, &style_bar_good, LV_PART_INDICATOR);

    // ─────────────────────────────────────────────────────────────────────
    // SENSOR KARTEN (CO2, Feinstaub, VOC)
    // ─────────────────────────────────────────────────────────────────────
    const char* labels[] = {"CO2", "Feinstaub", "VOC"};
    const char* units[]  = {"ppm", TXT_UNIT_PM, "idx"};
    const int card_widths[] = {113, 120, 100};
    const int card_start_x = CARD_START_X + CLIMATE_CARD_W + CARD_GAP;
    
    const lv_image_dsc_t* sensor_icons[] = {
        &emoji_icon_cloud,
        &emoji_icon_dash,
        &emoji_icon_nose
    };

    for (int i = 0; i < 3; i++) {
        int x = card_start_x;
        for(int j = 0; j < i; j++) x += card_widths[j] + CARD_GAP;

        s2_cards[i].width = card_widths[i];
        s2_cards[i].container = lv_obj_create(scr);
        lv_obj_add_style(s2_cards[i].container, &style_card, 0);
        lv_obj_set_size(s2_cards[i].container, card_widths[i], CARD_H);
        lv_obj_set_pos(s2_cards[i].container, x, CARD_Y);
        lv_obj_remove_flag(s2_cards[i].container, LV_OBJ_FLAG_SCROLLABLE);

        s2_cards[i].label = lv_label_create(s2_cards[i].container);
        lv_obj_set_style_text_font(s2_cards[i].label, FONT_12, 0);
        lv_obj_set_style_text_color(s2_cards[i].label, COLOR_TEXT_L, 0);
        lv_label_set_text(s2_cards[i].label, labels[i]);
        lv_obj_set_pos(s2_cards[i].label, 0, 0);

        s2_cards[i].emoji = lv_image_create(s2_cards[i].container);
        lv_image_set_src(s2_cards[i].emoji, sensor_icons[i]);
        lv_image_set_scale(s2_cards[i].emoji, 256);
        lv_obj_set_pos(s2_cards[i].emoji, card_widths[i] - 38, 0);

        s2_cards[i].value = lv_label_create(s2_cards[i].container);
        lv_obj_set_style_text_font(s2_cards[i].value, FONT_28, 0);
        lv_obj_set_style_text_color(s2_cards[i].value, COLOR_TEXT, 0);
        lv_label_set_text(s2_cards[i].value, "--");
        lv_obj_set_pos(s2_cards[i].value, 0, 32);

        s2_cards[i].unit = lv_label_create(s2_cards[i].container);
        lv_obj_set_style_text_font(s2_cards[i].unit, FONT_12, 0);
        lv_obj_set_style_text_color(s2_cards[i].unit, COLOR_TEXT_L, 0);
        lv_label_set_text(s2_cards[i].unit, units[i]);
        lv_obj_set_pos(s2_cards[i].unit, 40, 48);

        s2_cards[i].bar = lv_bar_create(s2_cards[i].container);
        lv_obj_set_size(s2_cards[i].bar, card_widths[i] - 24, 4);
        lv_obj_set_pos(s2_cards[i].bar, 0, CARD_H - 32);
        lv_bar_set_range(s2_cards[i].bar, 0, 100);
        lv_bar_set_value(s2_cards[i].bar, 33, LV_ANIM_OFF);
        lv_obj_add_style(s2_cards[i].bar, &style_bar_bg, LV_PART_MAIN);
        lv_obj_add_style(s2_cards[i].bar, &style_bar_good, LV_PART_INDICATOR);
    }

    Serial.println("[UI] Screen 2 (Detail) erstellt");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN UPDATE HELPER
 * ═══════════════════════════════════════════════════════════════════════════ */

static void update_screen1_time() {
    if (!s1_lbl_time) return;
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", cached_hour, cached_min);
    lv_label_set_text(s1_lbl_time, buf);
    
    if (s1_lbl_seconds) {
        char sec_buf[4];
        snprintf(sec_buf, sizeof(sec_buf), "%02d", cached_sec);
        lv_label_set_text(s1_lbl_seconds, sec_buf);
        
        lv_obj_update_layout(s1_lbl_time);
        int time_x = lv_obj_get_x(s1_lbl_time);
        int time_w = lv_obj_get_width(s1_lbl_time);
        lv_obj_set_pos(s1_lbl_seconds, time_x + time_w + 3, 65);
    }
    
    if (s1_lbl_date) {
        lv_label_set_text(s1_lbl_date, cached_date);
    }
}

static void update_screen2_time() {
    if (!s2_lbl_time) return;
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", cached_hour, cached_min);
    lv_label_set_text(s2_lbl_time, buf);
    
    if (s2_lbl_seconds) {
        char sec_buf[4];
        snprintf(sec_buf, sizeof(sec_buf), "%02d", cached_sec);
        lv_label_set_text(s2_lbl_seconds, sec_buf);
        
        lv_obj_update_layout(s2_lbl_time);
        int time_x = lv_obj_get_x(s2_lbl_time);
        int time_w = lv_obj_get_width(s2_lbl_time);
        lv_obj_set_pos(s2_lbl_seconds, time_x + time_w + 3, 65);
    }
    
    if (s2_lbl_date) {
        lv_label_set_text(s2_lbl_date, cached_date);
    }
}

static void update_screen1_sensors() {
    if (!s1_lbl_temp_value) return;
    
    char buf[16];
    Status air = get_air_quality(cached_co2, cached_pm25);
    
    // AQI aktualisieren
    lv_color_t col = get_status_color(air);
    lv_obj_set_style_arc_color(s1_arc_aqi, col, LV_PART_INDICATOR);
    int arc_val = (air == GOOD) ? 100 : (air == WARN) ? 66 : 33;
    lv_arc_set_value(s1_arc_aqi, arc_val);
    lv_label_set_text(s1_lbl_aqi_status, get_status_text(air));
    lv_obj_set_style_text_color(s1_lbl_aqi_status, col, 0);
    lv_image_set_src(s1_img_emoji, get_status_emoji_img(air));
    
    // Temperatur
    snprintf(buf, sizeof(buf), "%.1f", cached_temp);
    lv_label_set_text(s1_lbl_temp_value, buf);
    lv_obj_update_layout(s1_lbl_temp_value);
    lv_obj_set_pos(s1_lbl_temp_unit, lv_obj_get_x(s1_lbl_temp_value) + lv_obj_get_width(s1_lbl_temp_value) + 3, 48);
    
    Status temp_status = get_temp_status(cached_temp);
    lv_obj_remove_style(s1_bar_temp, NULL, LV_PART_INDICATOR);
    lv_obj_add_style(s1_bar_temp, get_bar_style(temp_status), LV_PART_INDICATOR);
    lv_bar_set_value(s1_bar_temp, (temp_status == GOOD) ? 33 : (temp_status == WARN) ? 66 : 100, LV_ANIM_ON);
    
    // Feuchte
    snprintf(buf, sizeof(buf), "%d", (int)cached_hum);
    lv_label_set_text(s1_lbl_hum_value, buf);
    lv_obj_update_layout(s1_lbl_hum_value);
    lv_obj_set_pos(s1_lbl_hum_unit, lv_obj_get_x(s1_lbl_hum_value) + lv_obj_get_width(s1_lbl_hum_value) + 3, 48);
    
    Status hum_status = get_hum_status(cached_hum);
    lv_obj_remove_style(s1_bar_hum, NULL, LV_PART_INDICATOR);
    lv_obj_add_style(s1_bar_hum, get_bar_style(hum_status), LV_PART_INDICATOR);
    lv_bar_set_value(s1_bar_hum, (hum_status == GOOD) ? 33 : (hum_status == WARN) ? 66 : 100, LV_ANIM_ON);
}

static void update_screen2_sensors() {
    if (!s2_lbl_temp_value || !s2_cards[0].value) return;
    
    char buf[16];
    
    // Temperatur
    snprintf(buf, sizeof(buf), "%.1f", cached_temp);
    lv_label_set_text(s2_lbl_temp_value, buf);
    lv_obj_update_layout(s2_lbl_temp_value);
    lv_obj_set_pos(s2_lbl_temp_unit, lv_obj_get_x(s2_lbl_temp_value) + lv_obj_get_width(s2_lbl_temp_value) + 3, 34);
    
    Status temp_status = get_temp_status(cached_temp);
    lv_obj_remove_style(s2_bar_temp, NULL, LV_PART_INDICATOR);
    lv_obj_add_style(s2_bar_temp, get_bar_style(temp_status), LV_PART_INDICATOR);
    lv_bar_set_value(s2_bar_temp, (temp_status == GOOD) ? 33 : (temp_status == WARN) ? 66 : 100, LV_ANIM_ON);
    
    // Feuchte
    snprintf(buf, sizeof(buf), "%d", (int)cached_hum);
    lv_label_set_text(s2_lbl_hum_value, buf);
    lv_obj_update_layout(s2_lbl_hum_value);
    lv_obj_set_pos(s2_lbl_hum_unit, lv_obj_get_x(s2_lbl_hum_value) + lv_obj_get_width(s2_lbl_hum_value) + 3, 96);
    
    Status hum_status = get_hum_status(cached_hum);
    lv_obj_remove_style(s2_bar_hum, NULL, LV_PART_INDICATOR);
    lv_obj_add_style(s2_bar_hum, get_bar_style(hum_status), LV_PART_INDICATOR);
    lv_bar_set_value(s2_bar_hum, (hum_status == GOOD) ? 33 : (hum_status == WARN) ? 66 : 100, LV_ANIM_ON);
    
    // CO2, PM2.5, VOC
    Status statuses[3] = {
        get_co2_status(cached_co2),
        get_pm25_status(cached_pm25),
        get_voc_status(cached_voc)
    };
    int values[3] = {cached_co2, cached_pm25, cached_voc};
    
    for (int i = 0; i < 3; i++) {
        snprintf(buf, sizeof(buf), "%d", values[i]);
        lv_label_set_text(s2_cards[i].value, buf);
        lv_obj_update_layout(s2_cards[i].value);
        lv_obj_set_pos(s2_cards[i].unit, lv_obj_get_x(s2_cards[i].value) + lv_obj_get_width(s2_cards[i].value) + 3, 48);
        
        lv_obj_remove_style(s2_cards[i].bar, NULL, LV_PART_INDICATOR);
        lv_obj_add_style(s2_cards[i].bar, get_bar_style(statuses[i]), LV_PART_INDICATOR);
        lv_bar_set_value(s2_cards[i].bar, (statuses[i] == GOOD) ? 33 : (statuses[i] == WARN) ? 66 : 100, LV_ANIM_ON);
    }
    
    // AQI
    Status air = get_air_quality(cached_co2, cached_pm25);
    lv_color_t col = get_status_color(air);
    lv_obj_set_style_arc_color(s2_arc_aqi, col, LV_PART_INDICATOR);
    lv_arc_set_value(s2_arc_aqi, (air == GOOD) ? 100 : (air == WARN) ? 66 : 33);
    lv_label_set_text(s2_lbl_aqi_status, get_status_text(air));
    lv_obj_set_style_text_color(s2_lbl_aqi_status, col, 0);
    lv_image_set_src(s2_img_emoji, get_status_emoji_img(air));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC API FUNKTIONEN
 * ═══════════════════════════════════════════════════════════════════════════ */

void ui_init() {
    Serial.println("[UI] Initialisiere Multi-Screen UI...");
    
    init_styles();
    
    // Beide Screens erstellen
    create_screen1();
    create_screen2();
    
    // Mit Screen 1 (Übersicht) starten
    current_screen = UI_SCREEN_OVERVIEW;
    lv_screen_load(screens[current_screen]);
    
    Serial.println("[UI] Multi-Screen UI initialisiert, Start mit Übersicht");
}

void ui_nextScreen() {
    Serial.println("[UI] ui_nextScreen() aufgerufen");
    Serial.flush();
    Serial.printf("[UI] Aktueller Screen: %d, screens[0]=%p, screens[1]=%p\n", 
                  current_screen, screens[0], screens[1]);
    Serial.flush();
    
    int next = (current_screen + 1) % UI_SCREEN_COUNT;
    Serial.printf("[UI] Wechsle zu Screen: %d\n", next);
    Serial.flush();
    ui_setScreen((UIScreen)next);
}

void ui_setScreen(UIScreen screen) {
    Serial.printf("[UI] ui_setScreen(%d) aufgerufen\n", screen);
    Serial.flush();
    
    if (screen >= UI_SCREEN_COUNT) {
        Serial.println("[UI] FEHLER: Screen-Index ungültig!");
        return;
    }
    if (screens[screen] == nullptr) {
        Serial.println("[UI] FEHLER: Screen ist NULL!");
        return;
    }
    
    Serial.println("[UI] Starte Screen-Wechsel...");
    Serial.flush();
    current_screen = screen;
    
    // Einfacher Load statt Animation (sicherer)
    Serial.println("[UI] Vor lv_screen_load()...");
    Serial.flush();
    lv_screen_load(screens[screen]);
    Serial.println("[UI] Nach lv_screen_load()");
    Serial.flush();
    
    // Aktuellen Screen mit gecachten Werten aktualisieren
    if (screen == UI_SCREEN_OVERVIEW) {
        Serial.println("[UI] Vor update_screen1_time()...");
        Serial.flush();
        update_screen1_time();
        Serial.println("[UI] Vor update_screen1_sensors()...");
        Serial.flush();
        update_screen1_sensors();
    } else {
        Serial.println("[UI] Vor update_screen2_time()...");
        Serial.flush();
        update_screen2_time();
        Serial.println("[UI] Vor update_screen2_sensors()...");
        Serial.flush();
        update_screen2_sensors();
    }
    
    Serial.printf("[UI] Wechsel zu Screen %d (%s) abgeschlossen\n", screen, 
                  screen == UI_SCREEN_OVERVIEW ? "Übersicht" : "Detail");
    Serial.flush();
}

UIScreen ui_getCurrentScreen() {
    return current_screen;
}

void ui_updateTime(int hour, int minute, int second) {
    // Cache aktualisieren
    cached_hour = hour;
    cached_min = minute;
    cached_sec = second;
    
    // Beide Screens aktualisieren (nur der aktive ist sichtbar)
    update_screen1_time();
    update_screen2_time();
}

void ui_updateDate(const char* date_str) {
    if (!date_str) return;
    strncpy(cached_date, date_str, sizeof(cached_date) - 1);
    cached_date[sizeof(cached_date) - 1] = '\0';
    
    if (s1_lbl_date) lv_label_set_text(s1_lbl_date, cached_date);
    if (s2_lbl_date) lv_label_set_text(s2_lbl_date, cached_date);
}

void ui_updateSensorValues(float temp, float hum, int co2, int pm25, int voc) {
    // Cache aktualisieren
    cached_temp = temp;
    cached_hum = hum;
    cached_co2 = co2;
    cached_pm25 = pm25;
    cached_voc = voc;
    
    Serial.printf("[UI] Update: T=%.1f H=%.0f CO2=%d PM=%d VOC=%d\n", temp, hum, co2, pm25, voc);
    
    // Beide Screens aktualisieren
    update_screen1_sensors();
    update_screen2_sensors();
}

void ui_updateSensors(const SensorReadings& readings) {
    ui_updateSensorValues(
        readings.aht.temperature,
        readings.aht.humidity,
        readings.mhz.co2_ppm,
        readings.pms.PM_AE_UG_2_5,
        readings.sgp.voc_index
    );
}
