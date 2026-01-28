/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LVGL 9 UI MANAGER
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Display: 480x320 (ST7796S)
 * Framework: LVGL 9.x
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
LV_IMAGE_DECLARE(emoji_icon_nose);  // VOC Icon

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
 * LAYOUT (480x320)
 * ═══════════════════════════════════════════════════════════════════════════ */
#define AQI_BOX_X       270
#define AQI_BOX_Y       38
#define AQI_BOX_W       190
#define AQI_BOX_H       90

#define CARD_Y          178
#define CARD_H          100
#define CARD_GAP        5
#define CARD_START_X    7

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBALE UI ELEMENTE
 * ═══════════════════════════════════════════════════════════════════════════ */
static lv_obj_t* lbl_time = nullptr;
static lv_obj_t* lbl_seconds = nullptr;
static lv_obj_t* lbl_date = nullptr;
static lv_obj_t* arc_aqi = nullptr;
static lv_obj_t* img_aqi_emoji = nullptr;
static lv_obj_t* lbl_aqi_title = nullptr;
static lv_obj_t* lbl_aqi_status = nullptr;

// Kombinierte Temp/Feuchte Kachel (links)
static lv_obj_t* card_climate = nullptr;
static lv_obj_t* lbl_temp_title = nullptr;
static lv_obj_t* img_temp = nullptr;
static lv_obj_t* lbl_temp_value = nullptr;
static lv_obj_t* lbl_temp_unit = nullptr;
static lv_obj_t* bar_temp = nullptr;
static lv_obj_t* lbl_hum_title = nullptr;
static lv_obj_t* img_hum = nullptr;
static lv_obj_t* lbl_hum_value = nullptr;
static lv_obj_t* lbl_hum_unit = nullptr;
static lv_obj_t* bar_hum = nullptr;

// Einzelne Kacheln (rechts)
struct SensorCard {
    lv_obj_t* container;
    lv_obj_t* label;
    lv_obj_t* emoji;
    lv_obj_t* value;
    lv_obj_t* unit;
    lv_obj_t* bar;
    int width;
};
static SensorCard cards[3];  // CO2, PM2.5, VOC

static lv_style_t style_card;
static lv_style_t style_bar_bg;
static lv_style_t style_bar_good;
static lv_style_t style_bar_warn;
static lv_style_t style_bar_bad;

/* ═══════════════════════════════════════════════════════════════════════════
 * STATUS HELPER
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

// WHO Indoor Air Quality Guidelines
static Status get_co2_status(int c) {
    if (c <= 1000) return GOOD;   // WHO: gut belüftet
    if (c <= 1500) return WARN;   // WHO: mäßig
    return BAD;                   // WHO: schlecht belüftet
}

// WHO PM2.5 Guidelines 2021 (24h Durchschnitt)
static Status get_pm25_status(int p) {
    if (p <= 15) return GOOD;     // WHO: Zielwert
    if (p <= 25) return WARN;     // WHO: Übergangsziel
    return BAD;                   // WHO: ungesund
}

// VOC Index (Sensirion SGP40)
static Status get_voc_status(int voc) {
    if (voc <= 150) return GOOD;   // Excellent - Good
    if (voc <= 250) return WARN;   // Moderate
    return BAD;                    // Unhealthy
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

static lv_style_t* get_bar_style(Status s) {
    if (s == GOOD) return &style_bar_good;
    if (s == WARN) return &style_bar_warn;
    return &style_bar_bad;
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
 * STYLES INITIALISIEREN
 * ═══════════════════════════════════════════════════════════════════════════ */
static void init_styles() {
    // Karten-Style
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, COLOR_CARD);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 12);
    lv_style_set_border_width(&style_card, 0);
    lv_style_set_outline_width(&style_card, 0);
    lv_style_set_shadow_width(&style_card, 12);
    lv_style_set_shadow_color(&style_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_card, LV_OPA_20);
    lv_style_set_shadow_offset_y(&style_card, 4);
    lv_style_set_pad_all(&style_card, 12);

    // Balken-Styles
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
 * UI ERSTELLEN
 * ═══════════════════════════════════════════════════════════════════════════ */
void ui_init() {
    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    init_styles();

    // UHRZEIT - fest zentriert links neben AQI Box
    lbl_time = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl_time, FONT_48, 0);
    lv_obj_set_style_text_color(lbl_time, COLOR_TEXT, 0);
    lv_label_set_text(lbl_time, "00:00");
    lv_obj_set_pos(lbl_time, 65, 55);

    // SEKUNDEN - direkt angrenzend an Uhrzeit, Unterkante auf gleicher Höhe
    lbl_seconds = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl_seconds, FONT_28, 0);
    lv_obj_set_style_text_color(lbl_seconds, COLOR_TEXT_L, 0);
    lv_label_set_text(lbl_seconds, "00");
    lv_obj_set_pos(lbl_seconds, 189, 65);  // Etwas nach rechts

    // DATUM
    lbl_date = lv_label_create(scr);
    lv_obj_set_style_text_font(lbl_date, FONT_16, 0);
    lv_obj_set_style_text_color(lbl_date, COLOR_DATE, 0);
    lv_label_set_text(lbl_date, "Di, 28. Jan");
    lv_obj_set_pos(lbl_date, 75, 115);

    // AQI BOX
    lv_obj_t* aqi_box = lv_obj_create(scr);
    lv_obj_add_style(aqi_box, &style_card, 0);
    lv_obj_set_size(aqi_box, AQI_BOX_W, AQI_BOX_H);
    lv_obj_set_pos(aqi_box, AQI_BOX_X, AQI_BOX_Y);
    lv_obj_remove_flag(aqi_box, LV_OBJ_FLAG_SCROLLABLE);

    // AQI Ring
    arc_aqi = lv_arc_create(aqi_box);
    lv_obj_set_size(arc_aqi, 60, 60);
    lv_obj_align(arc_aqi, LV_ALIGN_LEFT_MID, 5, 0);
    lv_arc_set_rotation(arc_aqi, 270);
    lv_arc_set_bg_angles(arc_aqi, 0, 360);
    lv_arc_set_range(arc_aqi, 0, 100);
    lv_arc_set_value(arc_aqi, 100);
    lv_obj_remove_style(arc_aqi, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arc_aqi, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_width(arc_aqi, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_aqi, COLOR_RING_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_aqi, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_aqi, COLOR_GOOD, LV_PART_INDICATOR);

    // Emoji im Ring
    img_aqi_emoji = lv_image_create(arc_aqi);
    lv_image_set_src(img_aqi_emoji, &emoji_happy);
    lv_obj_center(img_aqi_emoji);

    // Titel
    lbl_aqi_title = lv_label_create(aqi_box);
    lv_obj_set_style_text_font(lbl_aqi_title, FONT_12, 0);
    lv_obj_set_style_text_color(lbl_aqi_title, COLOR_TEXT_L, 0);
    lv_label_set_text(lbl_aqi_title, TXT_LUFTQUALITAET);
    lv_obj_align(lbl_aqi_title, LV_ALIGN_TOP_RIGHT, -5, 5);

    // Status Text
    lbl_aqi_status = lv_label_create(aqi_box);
    lv_obj_set_style_text_font(lbl_aqi_status, FONT_16, 0);
    lv_obj_set_style_text_color(lbl_aqi_status, COLOR_GOOD, 0);
    lv_label_set_text(lbl_aqi_status, TXT_SEHR_GUT);
    lv_obj_align(lbl_aqi_status, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

    // ═══════════════════════════════════════════════════════════════════════
    // KOMBINIERTE TEMP/FEUCHTE KACHEL (Links)
    // ═══════════════════════════════════════════════════════════════════════
    const int CLIMATE_CARD_W = 110;
    const int CLIMATE_CARD_H = 145;
    
    card_climate = lv_obj_create(scr);
    lv_obj_add_style(card_climate, &style_card, 0);
    lv_obj_set_size(card_climate, CLIMATE_CARD_W, CLIMATE_CARD_H);
    lv_obj_set_pos(card_climate, CARD_START_X, 155);
    lv_obj_remove_flag(card_climate, LV_OBJ_FLAG_SCROLLABLE);

    // --- TEMPERATUR (oben in der Kachel) ---
    lbl_temp_title = lv_label_create(card_climate);
    lv_obj_set_style_text_font(lbl_temp_title, FONT_12, 0);
    lv_obj_set_style_text_color(lbl_temp_title, COLOR_TEXT_L, 0);
    lv_label_set_text(lbl_temp_title, "Temp");
    lv_obj_set_pos(lbl_temp_title, 0, 0);

    img_temp = lv_image_create(card_climate);
    lv_image_set_src(img_temp, &emoji_icon_thermometer);
    lv_obj_set_pos(img_temp, CLIMATE_CARD_W - 38, 0);

    lbl_temp_value = lv_label_create(card_climate);
    lv_obj_set_style_text_font(lbl_temp_value, FONT_28, 0);
    lv_obj_set_style_text_color(lbl_temp_value, COLOR_TEXT, 0);
    lv_label_set_text(lbl_temp_value, "--");
    lv_obj_set_pos(lbl_temp_value, 0, 18);

    lbl_temp_unit = lv_label_create(card_climate);
    lv_obj_set_style_text_font(lbl_temp_unit, FONT_12, 0);
    lv_obj_set_style_text_color(lbl_temp_unit, COLOR_TEXT_L, 0);
    lv_label_set_text(lbl_temp_unit, TXT_UNIT_TEMP);
    lv_obj_set_pos(lbl_temp_unit, 50, 34);

    bar_temp = lv_bar_create(card_climate);
    lv_obj_set_size(bar_temp, CLIMATE_CARD_W - 24, 4);
    lv_obj_set_pos(bar_temp, 0, 52);
    lv_bar_set_range(bar_temp, 0, 100);
    lv_bar_set_value(bar_temp, 33, LV_ANIM_OFF);
    lv_obj_add_style(bar_temp, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(bar_temp, &style_bar_good, LV_PART_INDICATOR);

    // --- FEUCHTE (unten in der Kachel) ---
    lbl_hum_title = lv_label_create(card_climate);
    lv_obj_set_style_text_font(lbl_hum_title, FONT_12, 0);
    lv_obj_set_style_text_color(lbl_hum_title, COLOR_TEXT_L, 0);
    lv_label_set_text(lbl_hum_title, TXT_FEUCHTE);
    lv_obj_set_pos(lbl_hum_title, 0, 62);

    img_hum = lv_image_create(card_climate);
    lv_image_set_src(img_hum, &emoji_icon_droplet);
    lv_obj_set_pos(img_hum, CLIMATE_CARD_W - 38, 62);

    lbl_hum_value = lv_label_create(card_climate);
    lv_obj_set_style_text_font(lbl_hum_value, FONT_28, 0);
    lv_obj_set_style_text_color(lbl_hum_value, COLOR_TEXT, 0);
    lv_label_set_text(lbl_hum_value, "--");
    lv_obj_set_pos(lbl_hum_value, 0, 80);

    lbl_hum_unit = lv_label_create(card_climate);
    lv_obj_set_style_text_font(lbl_hum_unit, FONT_12, 0);
    lv_obj_set_style_text_color(lbl_hum_unit, COLOR_TEXT_L, 0);
    lv_label_set_text(lbl_hum_unit, "%");
    lv_obj_set_pos(lbl_hum_unit, 35, 96);

    bar_hum = lv_bar_create(card_climate);
    lv_obj_set_size(bar_hum, CLIMATE_CARD_W - 24, 4);
    lv_obj_set_pos(bar_hum, 0, 114);
    lv_bar_set_range(bar_hum, 0, 100);
    lv_bar_set_value(bar_hum, 33, LV_ANIM_OFF);
    lv_obj_add_style(bar_hum, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(bar_hum, &style_bar_good, LV_PART_INDICATOR);

    // ═══════════════════════════════════════════════════════════════════════
    // SENSOR KARTEN (Rechts: CO2, Feinstaub, VOC)
    // ═══════════════════════════════════════════════════════════════════════
    const char* labels[] = {"CO2", "Feinstaub", "VOC"};
    const char* units[]  = {"ppm", TXT_UNIT_PM, "idx"};
    const int card_widths[] = {113, 120, 100};
    const int card_start_x = CARD_START_X + CLIMATE_CARD_W + CARD_GAP;
    
    const lv_image_dsc_t* sensor_icons[] = {
        &emoji_icon_cloud,        // CO2
        &emoji_icon_dash,         // Feinstaub
        &emoji_icon_nose          // VOC
    };

    for (int i = 0; i < 3; i++) {
        int x = card_start_x;
        for(int j = 0; j < i; j++) x += card_widths[j] + CARD_GAP;

        cards[i].width = card_widths[i];
        cards[i].container = lv_obj_create(scr);
        lv_obj_add_style(cards[i].container, &style_card, 0);
        lv_obj_set_size(cards[i].container, card_widths[i], CARD_H);
        lv_obj_set_pos(cards[i].container, x, CARD_Y);
        lv_obj_remove_flag(cards[i].container, LV_OBJ_FLAG_SCROLLABLE);

        // Titel Label
        cards[i].label = lv_label_create(cards[i].container);
        lv_obj_set_style_text_font(cards[i].label, FONT_12, 0);
        lv_obj_set_style_text_color(cards[i].label, COLOR_TEXT_L, 0);
        lv_label_set_text(cards[i].label, labels[i]);
        lv_obj_set_pos(cards[i].label, 0, 0);

        // Sensor-Icon
        cards[i].emoji = lv_image_create(cards[i].container);
        lv_image_set_src(cards[i].emoji, sensor_icons[i]);
        lv_image_set_scale(cards[i].emoji, 256);
        lv_obj_set_pos(cards[i].emoji, card_widths[i] - 38, 0);

        // Wert
        cards[i].value = lv_label_create(cards[i].container);
        lv_obj_set_style_text_font(cards[i].value, FONT_28, 0);
        lv_obj_set_style_text_color(cards[i].value, COLOR_TEXT, 0);
        lv_label_set_text(cards[i].value, "--");
        lv_obj_set_pos(cards[i].value, 0, 32);

        // Einheit
        cards[i].unit = lv_label_create(cards[i].container);
        lv_obj_set_style_text_font(cards[i].unit, FONT_12, 0);
        lv_obj_set_style_text_color(cards[i].unit, COLOR_TEXT_L, 0);
        lv_label_set_text(cards[i].unit, units[i]);
        lv_obj_set_pos(cards[i].unit, 40, 48);

        // Status-Balken
        cards[i].bar = lv_bar_create(cards[i].container);
        lv_obj_set_size(cards[i].bar, card_widths[i] - 24, 4);
        lv_obj_set_pos(cards[i].bar, 0, CARD_H - 32);
        lv_bar_set_range(cards[i].bar, 0, 100);
        lv_bar_set_value(cards[i].bar, 33, LV_ANIM_OFF);
        lv_obj_add_style(cards[i].bar, &style_bar_bg, LV_PART_MAIN);
        lv_obj_add_style(cards[i].bar, &style_bar_good, LV_PART_INDICATOR);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * UPDATE FUNKTIONEN
 * ═══════════════════════════════════════════════════════════════════════════ */
void ui_updateTime(int hour, int minute, int second) {
    if (!lbl_time) return;
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
    lv_label_set_text(lbl_time, buf);
    
    // Sekunden separat aktualisieren
    if (lbl_seconds) {
        char sec_buf[4];
        snprintf(sec_buf, sizeof(sec_buf), "%02d", second);
        lv_label_set_text(lbl_seconds, sec_buf);
    }
}

void ui_updateDate(const char* date_str) {
    if (lbl_date && date_str) {
        lv_label_set_text(lbl_date, date_str);
    }
}

void ui_updateSensorValues(float temp, float hum, int co2, int pm25, int voc) {
    if (!lbl_temp_value || !cards[0].value) {
        Serial.println("[UI] ERROR: UI not initialized!");
        return;
    }
    
    Serial.printf("[UI] Updating: T=%.1f H=%.0f CO2=%d PM=%d VOC=%d\n", temp, hum, co2, pm25, voc);

    char buf[16];
    
    // ═══════════════════════════════════════════════════════════════════════
    // KLIMAKARTE (Temp + Feuchte)
    // ═══════════════════════════════════════════════════════════════════════
    
    // Temperatur
    snprintf(buf, sizeof(buf), "%.1f", temp);
    lv_label_set_text(lbl_temp_value, buf);
    lv_obj_invalidate(lbl_temp_value);
    lv_obj_update_layout(lbl_temp_value);
    lv_obj_set_pos(lbl_temp_unit, lv_obj_get_x(lbl_temp_value) + lv_obj_get_width(lbl_temp_value) + 3, 34);
    
    Status temp_status = get_temp_status(temp);
    lv_obj_remove_style(bar_temp, NULL, LV_PART_INDICATOR);
    lv_obj_add_style(bar_temp, get_bar_style(temp_status), LV_PART_INDICATOR);
    int temp_pct = (temp_status == GOOD) ? 33 : (temp_status == WARN) ? 66 : 100;
    lv_bar_set_value(bar_temp, temp_pct, LV_ANIM_ON);
    
    // Feuchte
    snprintf(buf, sizeof(buf), "%d", (int)hum);
    lv_label_set_text(lbl_hum_value, buf);
    lv_obj_invalidate(lbl_hum_value);
    lv_obj_update_layout(lbl_hum_value);
    lv_obj_set_pos(lbl_hum_unit, lv_obj_get_x(lbl_hum_value) + lv_obj_get_width(lbl_hum_value) + 3, 96);
    
    Status hum_status = get_hum_status(hum);
    lv_obj_remove_style(bar_hum, NULL, LV_PART_INDICATOR);
    lv_obj_add_style(bar_hum, get_bar_style(hum_status), LV_PART_INDICATOR);
    int hum_pct = (hum_status == GOOD) ? 33 : (hum_status == WARN) ? 66 : 100;
    lv_bar_set_value(bar_hum, hum_pct, LV_ANIM_ON);
    
    lv_obj_invalidate(card_climate);

    // ═══════════════════════════════════════════════════════════════════════
    // SENSOR KARTEN (CO2, Feinstaub, VOC)
    // ═══════════════════════════════════════════════════════════════════════
    
    Status statuses[3] = {
        get_co2_status(co2),
        get_pm25_status(pm25),
        get_voc_status(voc)
    };
    
    // CO2
    snprintf(buf, sizeof(buf), "%d", co2);
    lv_label_set_text(cards[0].value, buf);
    lv_obj_invalidate(cards[0].value);
    lv_obj_update_layout(cards[0].value);
    lv_obj_set_pos(cards[0].unit, lv_obj_get_x(cards[0].value) + lv_obj_get_width(cards[0].value) + 3, 48);
    
    // PM2.5
    snprintf(buf, sizeof(buf), "%d", pm25);
    lv_label_set_text(cards[1].value, buf);
    lv_obj_invalidate(cards[1].value);
    lv_obj_update_layout(cards[1].value);
    lv_obj_set_pos(cards[1].unit, lv_obj_get_x(cards[1].value) + lv_obj_get_width(cards[1].value) + 3, 48);
    
    // VOC
    snprintf(buf, sizeof(buf), "%d", voc);
    lv_label_set_text(cards[2].value, buf);
    lv_obj_invalidate(cards[2].value);
    lv_obj_update_layout(cards[2].value);
    lv_obj_set_pos(cards[2].unit, lv_obj_get_x(cards[2].value) + lv_obj_get_width(cards[2].value) + 3, 48);
    
    // Alle Karten invalidieren
    for (int i = 0; i < 3; i++) {
        lv_obj_invalidate(cards[i].container);
    }

    // Status-Balken aktualisieren
    for (int i = 0; i < 3; i++) {
        lv_obj_remove_style(cards[i].bar, NULL, LV_PART_INDICATOR);
        lv_obj_add_style(cards[i].bar, get_bar_style(statuses[i]), LV_PART_INDICATOR);
        int pct = (statuses[i] == GOOD) ? 33 : (statuses[i] == WARN) ? 66 : 100;
        lv_bar_set_value(cards[i].bar, pct, LV_ANIM_ON);
    }

    // Luftqualität (AQI) aktualisieren
    Status air = get_air_quality(co2, pm25);
    lv_color_t col = get_status_color(air);
    lv_obj_set_style_arc_color(arc_aqi, col, LV_PART_INDICATOR);
    int arc_val = (air == GOOD) ? 100 : (air == WARN) ? 66 : 33;
    lv_arc_set_value(arc_aqi, arc_val);
    lv_label_set_text(lbl_aqi_status, get_status_text(air));
    lv_obj_set_style_text_color(lbl_aqi_status, col, 0);
    lv_image_set_src(img_aqi_emoji, get_status_emoji_img(air));
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
