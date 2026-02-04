/**
 * ═══════════════════════════════════════════════════════════════════════════
 * INSPECTAIR - LVGL 9 UI MANAGER (Multi-Screen)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Display: 480x320 (ST7796S)
 * Framework: LVGL 9.x
 * 
 * Screen 1 (Overview): Large AQI on the right (full height), 2 large tiles on the left
 * Screen 2 (Detail):   Small AQI on top, 4 tiles on the bottom
 */

#include "ui_manager.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "colors.h"  // For unified threshold values

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

// Tree and leaves for tree animation
LV_IMAGE_DECLARE(img_tree_green);
LV_IMAGE_DECLARE(img_tree_yellow);
LV_IMAGE_DECLARE(img_tree_red);
LV_IMAGE_DECLARE(img_leaf_yellow);
LV_IMAGE_DECLARE(img_leaf_yellow_mirrored);
LV_IMAGE_DECLARE(img_leaf_red);

/* ═══════════════════════════════════════════════════════════════════════════
 * FONT KONFIGURATION
 * ═══════════════════════════════════════════════════════════════════════════ */
#define FONT_12  &ui_font_12
#define FONT_16  &ui_font_16
#define FONT_20  &ui_font_20
#define FONT_28  &ui_font_28
#define FONT_48  &ui_font_48

// Playfair Display fonts (Serif, for time/branding in analog screen)
LV_FONT_DECLARE(playfair_12);
LV_FONT_DECLARE(playfair_14);
LV_FONT_DECLARE(playfair_20);
LV_FONT_DECLARE(playfair_28);
LV_FONT_DECLARE(playfair_32);
LV_FONT_DECLARE(playfair_48);
#define FONT_PLAYFAIR_12  &playfair_12
#define FONT_PLAYFAIR_14  &playfair_14
#define FONT_PLAYFAIR_20  &playfair_20
#define FONT_PLAYFAIR_28  &playfair_28
#define FONT_PLAYFAIR_32  &playfair_32
#define FONT_PLAYFAIR_48  &playfair_48

// Orbitron fonts (Futuristic/Space, for bubble screen)
LV_FONT_DECLARE(orbitron_28);
LV_FONT_DECLARE(orbitron_16);
#define FONT_ORBITRON_28  &orbitron_28
#define FONT_ORBITRON_16  &orbitron_16

// Extended Montserrat fonts (mit µ, ³, ², °)
LV_FONT_DECLARE(ui_font_12_ext);
LV_FONT_DECLARE(ui_font_16_ext);
#define FONT_12_EXT  &ui_font_12_ext
#define FONT_16_EXT  &ui_font_16_ext

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
 * STATUS HELPER (shared) - Grenzwerte aus colors.h
 * ═══════════════════════════════════════════════════════════════════════════ */
enum Status { GOOD = 0, WARN = 1, BAD = 2 };

static Status get_temp_status(float t) {
    if (t >= LIMIT_TEMP_GOOD_MIN && t <= LIMIT_TEMP_GOOD_MAX) return GOOD;
    if (t >= LIMIT_TEMP_WARN_MIN && t <= LIMIT_TEMP_WARN_MAX) return WARN;
    return BAD;
}

static Status get_hum_status(float h) {
    if (h >= LIMIT_HUM_GOOD_MIN && h <= LIMIT_HUM_GOOD_MAX) return GOOD;
    if (h >= LIMIT_HUM_WARN_MIN && h <= LIMIT_HUM_WARN_MAX) return WARN;
    return BAD;
}

static Status get_co2_status(int c) {
    if (c < LIMIT_CO2_GOOD) return GOOD;
    if (c < LIMIT_CO2_BAD) return WARN;
    return BAD;
}

static Status get_pm25_status(int p) {
    if (p <= LIMIT_PM25_GOOD) return GOOD;
    if (p <= LIMIT_PM25_BAD) return WARN;
    return BAD;
}

static Status get_voc_status(int voc) {
    if (voc <= LIMIT_VOC_GOOD) return GOOD;
    if (voc <= LIMIT_VOC_BAD) return WARN;
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
static lv_obj_t* screens[UI_SCREEN_COUNT] = {nullptr, nullptr, nullptr, nullptr, nullptr};
static UIScreen current_screen = UI_SCREEN_TREE;

// Cached sensor values for screen updates
static float cached_temp = 0;
static float cached_hum = 0;
static int cached_co2 = 0;
static int cached_pm25 = 0;
static int cached_voc = 0;
static int cached_hour = 0;
static int cached_min = 0;
static int cached_sec = 0;
static char cached_date[24] = "Di, 28. Jan 2026";

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN 0: TREE ANIMATION (Start Screen)
 * ═══════════════════════════════════════════════════════════════════════════
 * Layout:
 * ┌──────────────────────────────────────────────────────────┐
 * │                                                          │
 * │              🌳 Animated Tree 🌳                        │
 * │              (Color based on air quality)                │
 * │                                                          │
 * │              🍂 Falling Leaves 🍂                       │
 * │              (when yellow/red)                           │
 * │                                                          │
 * └──────────────────────────────────────────────────────────┘
 */

// Screen 0 (Tree) UI elements
static lv_obj_t* s0_tree_img = nullptr;
static lv_obj_t* s0_leaf_yellow_1 = nullptr;
static lv_obj_t* s0_leaf_yellow_2 = nullptr;
static lv_obj_t* s0_leaf_red = nullptr;
static int s0_leaf_end_y = 290;  // Ground Y position for 320px height
static Status s0_last_air_status = GOOD;

// Forward declarations for leaf animations
static void s0_animate_yellow_leaf_from_branch(lv_obj_t* leaf, float branch_x_ratio, float branch_y_ratio, int sway, uint32_t delay_ms, int end_y_offset);
static void s0_animate_red_leaf_from_branch(lv_obj_t* leaf);
static void s0_animate_fade_in(lv_obj_t* obj, uint32_t duration_ms);

// ------------------------------------------------------------
// Callback wenn Blatt unten angekommen ist - startet von neuer Position
// ------------------------------------------------------------
static void s0_leaf_fall_complete_cb(lv_anim_t* a)
{
    lv_obj_t* leaf = (lv_obj_t*)a->var;

    // Only restart if the leaf is still visible (state hasn't changed)
    if(!lv_obj_has_flag(leaf, LV_OBJ_FLAG_HIDDEN)) {
        // Stop all running animations for this leaf
        lv_anim_delete(leaf, NULL);

        // Restart yellow leaves on fixed paths
        if(leaf == s0_leaf_yellow_1) {
            s0_animate_yellow_leaf_from_branch(leaf, 0.36f, 0.31f, 14, 0, -12);
        } else if(leaf == s0_leaf_yellow_2) {
            s0_animate_yellow_leaf_from_branch(leaf, 0.66f, 0.36f, 22, 650, -24);
        } else {
            // Rotes Blatt: von der Astspitze starten
            s0_animate_red_leaf_from_branch(leaf);
        }
    }
}

// ------------------------------------------------------------
// Yellow leaf animation: branch tip calculated, custom path + delay
// ------------------------------------------------------------
static void s0_animate_yellow_leaf_from_branch(lv_obj_t* leaf, float branch_x_ratio, float branch_y_ratio, int sway, uint32_t delay_ms, int end_y_offset)
{
    if (!s0_tree_img) return;
    
    // Get tree position and size
    int tree_x = lv_obj_get_x(s0_tree_img);
    int tree_y = lv_obj_get_y(s0_tree_img);
    int tree_w = lv_obj_get_width(s0_tree_img);
    int tree_h = lv_obj_get_height(s0_tree_img);

    int tip_x = tree_x + (int)(tree_w * branch_x_ratio);
    int tip_y = tree_y + (int)(tree_h * branch_y_ratio);

    int leaf_w = lv_obj_get_width(leaf);
    int leaf_h = lv_obj_get_height(leaf);
    int start_x = tip_x - (leaf_w / 2);
    int start_y = tip_y - (leaf_h / 2);
    int end_y = s0_leaf_end_y + end_y_offset;

    lv_obj_set_pos(leaf, start_x, start_y);
    lv_obj_remove_flag(leaf, LV_OBJ_FLAG_HIDDEN);

    const uint32_t hang_ms = 3500;

    // Vertikaler Fall
    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, leaf);
    lv_anim_set_values(&a_y, start_y, end_y);
    lv_anim_set_duration(&a_y, 4500 + (rand() % 1200));
    lv_anim_set_delay(&a_y, hang_ms + delay_ms);
    lv_anim_set_path_cb(&a_y, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a_y, [](void* obj, int32_t v) { lv_obj_set_y((lv_obj_t*)obj, v); });
    lv_anim_set_ready_cb(&a_y, s0_leaf_fall_complete_cb);
    lv_anim_start(&a_y);

    // Horizontaler Wind (eigener Pfad)
    lv_anim_t a_x;
    lv_anim_init(&a_x);
    lv_anim_set_var(&a_x, leaf);
    lv_anim_set_values(&a_x, start_x - sway, start_x + sway);
    lv_anim_set_duration(&a_x, 900 + (rand() % 500));
    lv_anim_set_playback_duration(&a_x, 900 + (rand() % 500));
    lv_anim_set_repeat_count(&a_x, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_delay(&a_x, hang_ms + delay_ms);
    lv_anim_set_path_cb(&a_x, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a_x, [](void* obj, int32_t v) { lv_obj_set_x((lv_obj_t*)obj, v); });
    lv_anim_start(&a_x);

    // Leichte Rotation
    lv_anim_t a_rot;
    lv_anim_init(&a_rot);
    lv_anim_set_var(&a_rot, leaf);
    lv_anim_set_values(&a_rot, -25, 25);
    lv_anim_set_duration(&a_rot, 800 + (rand() % 400));
    lv_anim_set_playback_duration(&a_rot, 800 + (rand() % 400));
    lv_anim_set_repeat_count(&a_rot, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_delay(&a_rot, hang_ms + delay_ms);
    lv_anim_set_exec_cb(&a_rot, [](void* obj, int32_t v) { lv_image_set_rotation((lv_obj_t*)obj, v * 10); });
    lv_anim_start(&a_rot);
}

// ------------------------------------------------------------
// Red leaf: start from branch tip (position calculated from tree size)
// ------------------------------------------------------------
static void s0_animate_red_leaf_from_branch(lv_obj_t* leaf)
{
    if (!s0_tree_img) return;
    
    // Get tree position and size
    int tree_x = lv_obj_get_x(s0_tree_img);
    int tree_y = lv_obj_get_y(s0_tree_img);
    int tree_w = lv_obj_get_width(s0_tree_img);
    int tree_h = lv_obj_get_height(s0_tree_img);

    // Astspitze relativ zur Baumgrafik
    const float BRANCH_TIP_X_RATIO = 0.84f;
    const float BRANCH_TIP_Y_RATIO = 0.40f;

    int tip_x = tree_x + (int)(tree_w * BRANCH_TIP_X_RATIO);
    int tip_y = tree_y + (int)(tree_h * BRANCH_TIP_Y_RATIO);

    // Blatt auf Astspitze zentrieren
    int leaf_w = lv_obj_get_width(leaf);
    int leaf_h = lv_obj_get_height(leaf);
    int start_x = tip_x - (leaf_w / 2);
    int start_y = tip_y - (leaf_h / 2);
    int end_y = s0_leaf_end_y - 7;

    lv_obj_set_pos(leaf, start_x, start_y);
    lv_obj_remove_flag(leaf, LV_OBJ_FLAG_HIDDEN);

    // Vertikaler Fall
    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, leaf);
    lv_anim_set_values(&a_y, start_y, end_y);
    lv_anim_set_duration(&a_y, 3000 + (rand() % 1500));
    lv_anim_set_delay(&a_y, 3500);
    lv_anim_set_path_cb(&a_y, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a_y, [](void* obj, int32_t v) { lv_obj_set_y((lv_obj_t*)obj, v); });
    lv_anim_set_ready_cb(&a_y, s0_leaf_fall_complete_cb);
    lv_anim_start(&a_y);

    // Horizontaler Wind
    lv_anim_t a_x;
    lv_anim_init(&a_x);
    lv_anim_set_var(&a_x, leaf);
    lv_anim_set_values(&a_x, start_x - 15, start_x + 15);
    lv_anim_set_duration(&a_x, 600 + (rand() % 400));
    lv_anim_set_playback_duration(&a_x, 600 + (rand() % 400));
    lv_anim_set_delay(&a_x, 3500);
    lv_anim_set_repeat_count(&a_x, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a_x, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a_x, [](void* obj, int32_t v) { lv_obj_set_x((lv_obj_t*)obj, v); });
    lv_anim_start(&a_x);

    // Leichte Rotation
    lv_anim_t a_rot;
    lv_anim_init(&a_rot);
    lv_anim_set_var(&a_rot, leaf);
    lv_anim_set_values(&a_rot, -25, 25);
    lv_anim_set_duration(&a_rot, 600 + (rand() % 300));
    lv_anim_set_playback_duration(&a_rot, 600 + (rand() % 300));
    lv_anim_set_delay(&a_rot, 3500);
    lv_anim_set_repeat_count(&a_rot, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a_rot, [](void* obj, int32_t v) { lv_image_set_rotation((lv_obj_t*)obj, v * 10); });
    lv_anim_start(&a_rot);
}

// ------------------------------------------------------------
// Smooth Fade-In
// ------------------------------------------------------------
static void s0_animate_fade_in(lv_obj_t* obj, uint32_t duration_ms)
{
    lv_obj_set_style_opa(obj, LV_OPA_TRANSP, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, [](void* o, int32_t v) {
        lv_obj_set_style_opa((lv_obj_t*)o, v, 0);
    });
    lv_anim_start(&a);
}

// ------------------------------------------------------------
// Tree state change based on air quality
// ------------------------------------------------------------
static void s0_show_green()
{
    if (!s0_tree_img) return;
    
    lv_image_set_src(s0_tree_img, &img_tree_green);

    // Stop animations and hide leaves
    if (s0_leaf_yellow_1) {
        lv_anim_delete(s0_leaf_yellow_1, NULL);
        lv_obj_add_flag(s0_leaf_yellow_1, LV_OBJ_FLAG_HIDDEN);
    }
    if (s0_leaf_yellow_2) {
        lv_anim_delete(s0_leaf_yellow_2, NULL);
        lv_obj_add_flag(s0_leaf_yellow_2, LV_OBJ_FLAG_HIDDEN);
    }
    if (s0_leaf_red) {
        lv_anim_delete(s0_leaf_red, NULL);
        lv_obj_add_flag(s0_leaf_red, LV_OBJ_FLAG_HIDDEN);
    }
}

static void s0_show_yellow()
{
    if (!s0_tree_img) return;
    
    lv_image_set_src(s0_tree_img, &img_tree_yellow);

    // Alte Animationen stoppen
    if (s0_leaf_yellow_1) lv_anim_delete(s0_leaf_yellow_1, NULL);
    if (s0_leaf_yellow_2) lv_anim_delete(s0_leaf_yellow_2, NULL);
    if (s0_leaf_red) {
        lv_anim_delete(s0_leaf_red, NULL);
        lv_obj_add_flag(s0_leaf_red, LV_OBJ_FLAG_HIDDEN);
    }

    // Two yellow leaves: branch tips left/right, staggered, different paths
    if (s0_leaf_yellow_1) {
        s0_animate_yellow_leaf_from_branch(s0_leaf_yellow_1, 0.36f, 0.31f, 14, 0, -12);
        s0_animate_fade_in(s0_leaf_yellow_1, 500);
    }
    if (s0_leaf_yellow_2) {
        s0_animate_yellow_leaf_from_branch(s0_leaf_yellow_2, 0.66f, 0.36f, 22, 650, -24);
        s0_animate_fade_in(s0_leaf_yellow_2, 500);
    }
}

static void s0_show_red()
{
    if (!s0_tree_img) return;
    
    lv_image_set_src(s0_tree_img, &img_tree_red);

    // Alte Animationen stoppen
    if (s0_leaf_yellow_1) {
        lv_anim_delete(s0_leaf_yellow_1, NULL);
        lv_obj_add_flag(s0_leaf_yellow_1, LV_OBJ_FLAG_HIDDEN);
    }
    if (s0_leaf_yellow_2) {
        lv_anim_delete(s0_leaf_yellow_2, NULL);
        lv_obj_add_flag(s0_leaf_yellow_2, LV_OBJ_FLAG_HIDDEN);
    }

    // Rotes Blatt animieren: Start an der Astspitze
    if (s0_leaf_red) {
        lv_anim_delete(s0_leaf_red, NULL);
        s0_animate_red_leaf_from_branch(s0_leaf_red);
        s0_animate_fade_in(s0_leaf_red, 500);
    }
}

// ------------------------------------------------------------
// Baum-Screen erstellen
// ------------------------------------------------------------
static void create_screen0_tree() {
    screens[UI_SCREEN_TREE] = lv_obj_create(NULL);
    lv_obj_t* scr = screens[UI_SCREEN_TREE];
    
    // White background
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Display size: 480x320
    const int disp_w = 480;
    const int disp_h = 320;
    
    // Baum-Bild erstellen und zentrieren
    s0_tree_img = lv_image_create(scr);
    lv_image_set_src(s0_tree_img, &img_tree_green);
    lv_image_set_scale(s0_tree_img, 320);  // ~125% size (256 = 100%)
    
    // Update layout to get size
    lv_obj_update_layout(s0_tree_img);
    int tree_w = lv_obj_get_width(s0_tree_img);
    int tree_h = lv_obj_get_height(s0_tree_img);
    const int extra_down_px = 20;
    
    lv_obj_set_pos(s0_tree_img, (disp_w - tree_w) / 2, disp_h - tree_h + extra_down_px);
    
    // Calculate leaf pile position
    const int leaf_pile_offset = 30;
    s0_leaf_end_y = disp_h - leaf_pile_offset;

    // Gelbes Blatt 1 (links)
    s0_leaf_yellow_1 = lv_image_create(scr);
    lv_image_set_src(s0_leaf_yellow_1, &img_leaf_yellow);
    lv_image_set_scale(s0_leaf_yellow_1, 48);  // ~19% size
    lv_obj_add_flag(s0_leaf_yellow_1, LV_OBJ_FLAG_HIDDEN);

    // Gelbes Blatt 2 (rechts, gespiegelt)
    s0_leaf_yellow_2 = lv_image_create(scr);
    lv_image_set_src(s0_leaf_yellow_2, &img_leaf_yellow_mirrored);
    lv_image_set_scale(s0_leaf_yellow_2, 48);
    lv_obj_add_flag(s0_leaf_yellow_2, LV_OBJ_FLAG_HIDDEN);

    // Rotes Blatt
    s0_leaf_red = lv_image_create(scr);
    lv_image_set_src(s0_leaf_red, &img_leaf_red);
    lv_image_set_scale(s0_leaf_red, 48);
    lv_obj_add_flag(s0_leaf_red, LV_OBJ_FLAG_HIDDEN);

    // Initial: green tree (good air quality)
    s0_last_air_status = GOOD;
    s0_show_green();

    Serial.println("[UI] Screen 0 (tree animation) created");
}

// ------------------------------------------------------------
// Update tree screen based on air quality
// ------------------------------------------------------------
static void update_screen0_tree() {
    Status air = get_air_quality(cached_co2, cached_pm25);
    
    // Only update when state has changed
    if (air != s0_last_air_status) {
        s0_last_air_status = air;
        
        switch (air) {
            case GOOD:
                s0_show_green();
                break;
            case WARN:
                s0_show_yellow();
                break;
            case BAD:
                s0_show_red();
                break;
        }
        
        Serial.printf("[UI] Tree state changed: %s\n", 
                      air == GOOD ? "GREEN" : (air == WARN ? "YELLOW" : "RED"));
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN 1: OVERVIEW (Large AQI + 2 large tiles)
 * ═══════════════════════════════════════════════════════════════════════════
 * Layout:
 * ┌──────────────────────────────────────────────────────┐
 * │  Clock + Date (top left)     │   Large AQI Box      │
 * │  12:34 56                    │   (full height)      │
 * │  Tue, Jan 28                 │                      │
 * ├──────────────────────────────┤   🙂 Air Quality    │
 * │    TEMPERATURE   │  HUMIDITY │      Very good       │
 * │    🌡️  23.5°C    │  💧 45%  │                      │
 * │    ═══════       │  ═══════  │   Arc + Emoji        │
 * └──────────────────────────────┴──────────────────────┘
 */

// Screen 1 UI elements
static lv_obj_t* s1_lbl_time = nullptr;
static lv_obj_t* s1_lbl_seconds = nullptr;
static lv_obj_t* s1_lbl_date = nullptr;
static lv_obj_t* s1_aqi_box = nullptr;
static lv_obj_t* s1_arc_aqi = nullptr;
static lv_obj_t* s1_img_emoji = nullptr;
static lv_obj_t* s1_lbl_aqi_title = nullptr;
static lv_obj_t* s1_lbl_aqi_status = nullptr;

// 2 large tiles
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

    // Layout constants for screen 1
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
    // LARGE AQI BOX (right side, full height)
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

    // Large AQI ring (centered)
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

    // Large emoji in ring
    s1_img_emoji = lv_image_create(s1_arc_aqi);
    lv_image_set_src(s1_img_emoji, &emoji_happy);
    lv_image_set_scale(s1_img_emoji, 384);  // 1.5x larger
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
    lv_obj_set_style_text_font(s1_lbl_hum_value, FONT_28, 0);  // FONT_28 for consistency
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

    Serial.println("[UI] Screen 1 (overview) created");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN 2: DETAIL (Small AQI + 4 tiles)
 * ═══════════════════════════════════════════════════════════════════════════
 * Layout:
 * ┌────────────────────────────────────────────────────────┐
 * │  Clock + Date        │  Small AQI Box                 │
 * │  12:34 56            │  🙂 Air Quality Very good      │
 * │  Tue, Jan 28         │                                 │
 * ├──────────┬───────────┼─────────────┬──────────────────┤
 * │ Temp/Hum │    CO2    │   PM2.5     │       VOC        │
 * │ combined │           │             │                  │
 * └──────────┴───────────┴─────────────┴──────────────────┘
 */

// Screen 2 UI elements
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
 * SCREEN 3: ANALOG COCKPIT (Instrumente mit Zeigern)
 * ═══════════════════════════════════════════════════════════════════════════
 * Layout:
 * ┌──────────────────────────────────────────────────────────┐
 * │                    INSPECTAIR                            │
 * │                     14:32                                │
 * │                   Fr, 31. Jan                            │
 * ├──────────────────────────────────────────────────────────┤
 * │   [Temp °C]      [   CO2 ppm   ]      [Feuchte %]       │
 * │    (klein)          (GROSS)            (klein)          │
 * └──────────────────────────────────────────────────────────┘
 */

// Analog Screen Farben
#define COLOR_ANALOG_BG      lv_color_hex(0x2c3e50)
#define COLOR_ANALOG_BORDER  lv_color_hex(0x8b7355)
#define COLOR_ANALOG_BEIGE   lv_color_hex(0xf5f5dc)
#define COLOR_ANALOG_GRAY    lv_color_hex(0xbbbbbb)
#define COLOR_ANALOG_DGRAY   lv_color_hex(0x888888)
#define COLOR_GAUGE_BG       lv_color_hex(0x2a2a2a)
#define COLOR_NEEDLE         lv_color_hex(0xc0392b)
#define COLOR_TEMP_GAUGE     lv_color_hex(0xe74c3c)
#define COLOR_CO2_GAUGE      lv_color_hex(0x9b59b6)
#define COLOR_HUM_GAUGE      lv_color_hex(0x3498db)

// Gauge structure for analog screen
typedef struct {
    lv_obj_t* container;
    lv_obj_t* arc_bg;
    lv_obj_t* arc_fill;
    lv_obj_t* needle;
    lv_obj_t* needle_center;
    lv_obj_t* tick_lines[12];
    lv_point_precise_t tick_points[12][2];
    lv_obj_t* value_label;
    lv_obj_t* name_label;
    float current_value;
    float min_val;
    float max_val;
    lv_color_t color;
    const char* unit;
    const char* name;
    bool is_big;
    int cx, cy;
    int needle_len;
    int tick_count;
} AnalogGauge;

// Screen 3 UI Elemente
static lv_obj_t* s3_lbl_time = nullptr;
static lv_obj_t* s3_lbl_date = nullptr;
static lv_obj_t* s3_lbl_brand = nullptr;

static AnalogGauge s3_gauge_temp;
static AnalogGauge s3_gauge_co2;
static AnalogGauge s3_gauge_hum;

// Zeiger Punkte
static lv_point_precise_t s3_needle_pts_temp[2];
static lv_point_precise_t s3_needle_pts_co2[2];
static lv_point_precise_t s3_needle_pts_hum[2];

// ------------------------------------------------------------
// Gauge Update Funktion
// ------------------------------------------------------------
static void s3_update_gauge(AnalogGauge* g, lv_point_precise_t* pts) {
    float clampedValue = g->current_value;
    if (clampedValue < g->min_val) clampedValue = g->min_val;
    if (clampedValue > g->max_val) clampedValue = g->max_val;
    float ratio = (clampedValue - g->min_val) / (g->max_val - g->min_val);

    // Arc Update
    lv_arc_set_value(g->arc_fill, (int)(ratio * 100));

    // Zeiger Update (180° = links, 360° = rechts)
    float needle_angle_deg = 180.0f + ratio * 180.0f;
    float needle_rad = needle_angle_deg * (float)M_PI / 180.0f;

    int nx = g->cx + (int)(g->needle_len * cosf(needle_rad));
    int ny = g->cy + (int)(g->needle_len * sinf(needle_rad));

    pts[0].x = g->cx;
    pts[0].y = g->cy;
    pts[1].x = nx;
    pts[1].y = ny;
    lv_line_set_points(g->needle, pts, 2);

    // Wert Label
    char buf[16];
    if (g->is_big) {
        snprintf(buf, sizeof(buf), "%d %s", (int)g->current_value, g->unit);
    } else {
        snprintf(buf, sizeof(buf), "%.1f %s", g->current_value, g->unit);
    }
    lv_label_set_text(g->value_label, buf);
}

// ------------------------------------------------------------
// Kleine Gauge erstellen (Temperatur / Feuchtigkeit)
// ------------------------------------------------------------
static void s3_create_small_gauge(lv_obj_t* parent, AnalogGauge* g, int x, int y,
                                  float min_val, float max_val, lv_color_t color,
                                  const char* unit, const char* name, lv_point_precise_t* pts) {
    g->min_val = min_val;
    g->max_val = max_val;
    g->color = color;
    g->unit = unit;
    g->name = name;
    g->current_value = min_val;
    g->is_big = false;
    g->cx = 50;
    g->cy = 55;
    g->needle_len = 36;
    g->tick_count = 9;

    // Container
    g->container = lv_obj_create(parent);
    lv_obj_remove_style_all(g->container);
    lv_obj_set_size(g->container, 100, 110);
    lv_obj_set_pos(g->container, x, y);
    lv_obj_clear_flag(g->container, LV_OBJ_FLAG_SCROLLABLE);

    // Hintergrund Arc
    g->arc_bg = lv_arc_create(g->container);
    lv_obj_set_size(g->arc_bg, 80, 80);
    lv_obj_set_pos(g->arc_bg, 10, 15);
    lv_arc_set_rotation(g->arc_bg, 180);
    lv_arc_set_bg_angles(g->arc_bg, 0, 180);
    lv_arc_set_value(g->arc_bg, 0);
    lv_obj_remove_style(g->arc_bg, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(g->arc_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(g->arc_bg, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_color(g->arc_bg, COLOR_GAUGE_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(g->arc_bg, false, LV_PART_MAIN);
    lv_obj_set_style_arc_width(g->arc_bg, 0, LV_PART_INDICATOR);

    // Farbiger Arc
    g->arc_fill = lv_arc_create(g->container);
    lv_obj_set_size(g->arc_fill, 80, 80);
    lv_obj_set_pos(g->arc_fill, 10, 15);
    lv_arc_set_rotation(g->arc_fill, 180);
    lv_arc_set_bg_angles(g->arc_fill, 0, 180);
    lv_arc_set_range(g->arc_fill, 0, 100);
    lv_arc_set_value(g->arc_fill, 50);
    lv_obj_remove_style(g->arc_fill, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(g->arc_fill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(g->arc_fill, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(g->arc_fill, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(g->arc_fill, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(g->arc_fill, false, LV_PART_INDICATOR);

    // Skalenstriche
    for (int i = 0; i < g->tick_count; i++) {
        float ratio = (float)i / (g->tick_count - 1);
        float angle_deg = 180.0f + ratio * 180.0f;
        float angle_rad = angle_deg * (float)M_PI / 180.0f;

        bool major = (i % 2 == 0);
        int outer = 40;
        int inner = major ? 32 : 34;
        int width = major ? 1 : 1;

        int x1 = g->cx + (int)(inner * cosf(angle_rad));
        int y1 = g->cy + (int)(inner * sinf(angle_rad));
        int x2 = g->cx + (int)(outer * cosf(angle_rad));
        int y2 = g->cy + (int)(outer * sinf(angle_rad));

        g->tick_points[i][0].x = x1;
        g->tick_points[i][0].y = y1;
        g->tick_points[i][1].x = x2;
        g->tick_points[i][1].y = y2;

        g->tick_lines[i] = lv_line_create(g->container);
        lv_obj_set_style_line_width(g->tick_lines[i], width, 0);
        lv_obj_set_style_line_color(g->tick_lines[i], COLOR_ANALOG_DGRAY, 0);
        lv_line_set_points(g->tick_lines[i], g->tick_points[i], 2);
    }

    // Zeiger
    g->needle = lv_line_create(g->container);
    lv_obj_set_style_line_width(g->needle, 2, 0);
    lv_obj_set_style_line_color(g->needle, COLOR_NEEDLE, 0);
    lv_obj_set_style_line_rounded(g->needle, true, 0);
    pts[0].x = g->cx;
    pts[0].y = g->cy;
    pts[1].x = g->cx - g->needle_len;
    pts[1].y = g->cy;
    lv_line_set_points(g->needle, pts, 2);

    // Zeiger Mittelpunkt
    g->needle_center = lv_obj_create(g->container);
    lv_obj_remove_style_all(g->needle_center);
    lv_obj_set_size(g->needle_center, 10, 10);
    lv_obj_set_pos(g->needle_center, g->cx - 5, g->cy - 5);
    lv_obj_set_style_radius(g->needle_center, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(g->needle_center, COLOR_NEEDLE, 0);
    lv_obj_set_style_bg_opa(g->needle_center, LV_OPA_COVER, 0);

    // Wert Label (Playfair 20 mit ° Symbol)
    g->value_label = lv_label_create(g->container);
    lv_obj_set_style_text_font(g->value_label, FONT_PLAYFAIR_20, 0);
    lv_obj_set_style_text_color(g->value_label, COLOR_CARD, 0);
    lv_obj_set_style_text_align(g->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(g->value_label, 100);
    lv_obj_set_pos(g->value_label, 0, 62);  // Centered between gauge (cy=50) and name
    lv_label_set_text(g->value_label, "--");

    // Name label (Playfair Serif for elegant look)
    g->name_label = lv_label_create(g->container);
    lv_obj_set_style_text_font(g->name_label, FONT_PLAYFAIR_14, 0);
    lv_obj_set_style_text_color(g->name_label, COLOR_ANALOG_DGRAY, 0);
    lv_obj_set_style_text_align(g->name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(g->name_label, 100);
    lv_obj_set_pos(g->name_label, 0, 84);  // Below the value
    lv_label_set_text(g->name_label, name);
}

// ------------------------------------------------------------
// Create large gauge (CO2)
// ------------------------------------------------------------
static void s3_create_big_gauge(lv_obj_t* parent, AnalogGauge* g, int x, int y,
                                float min_val, float max_val, lv_color_t color,
                                const char* unit, const char* name, lv_point_precise_t* pts) {
    g->min_val = min_val;
    g->max_val = max_val;
    g->color = color;
    g->unit = unit;
    g->name = name;
    g->current_value = min_val;
    g->is_big = true;
    g->cx = 110;
    g->cy = 110;
    g->needle_len = 82;
    g->tick_count = 11;

    // Container
    g->container = lv_obj_create(parent);
    lv_obj_remove_style_all(g->container);
    lv_obj_set_size(g->container, 220, 195);
    lv_obj_set_pos(g->container, x, y);
    lv_obj_clear_flag(g->container, LV_OBJ_FLAG_SCROLLABLE);

    // Hintergrund Arc
    g->arc_bg = lv_arc_create(g->container);
    lv_obj_set_size(g->arc_bg, 180, 180);
    lv_obj_set_pos(g->arc_bg, 20, 20);
    lv_arc_set_rotation(g->arc_bg, 180);
    lv_arc_set_bg_angles(g->arc_bg, 0, 180);
    lv_arc_set_value(g->arc_bg, 0);
    lv_obj_remove_style(g->arc_bg, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(g->arc_bg, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(g->arc_bg, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_color(g->arc_bg, COLOR_GAUGE_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(g->arc_bg, false, LV_PART_MAIN);
    lv_obj_set_style_arc_width(g->arc_bg, 0, LV_PART_INDICATOR);

    // Farbiger Arc
    g->arc_fill = lv_arc_create(g->container);
    lv_obj_set_size(g->arc_fill, 180, 180);
    lv_obj_set_pos(g->arc_fill, 20, 20);
    lv_arc_set_rotation(g->arc_fill, 180);
    lv_arc_set_bg_angles(g->arc_fill, 0, 180);
    lv_arc_set_range(g->arc_fill, 0, 100);
    lv_arc_set_value(g->arc_fill, 50);
    lv_obj_remove_style(g->arc_fill, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(g->arc_fill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(g->arc_fill, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_width(g->arc_fill, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(g->arc_fill, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(g->arc_fill, false, LV_PART_INDICATOR);

    // Skalenstriche
    for (int i = 0; i < g->tick_count; i++) {
        float ratio = (float)i / (g->tick_count - 1);
        float angle_deg = 180.0f + ratio * 180.0f;
        float angle_rad = angle_deg * (float)M_PI / 180.0f;

        bool major = (i % 2 == 0);
        int outer = 90;
        int inner = major ? 76 : 81;
        int width = major ? 2 : 1;

        int x1 = g->cx + (int)(inner * cosf(angle_rad));
        int y1 = g->cy + (int)(inner * sinf(angle_rad));
        int x2 = g->cx + (int)(outer * cosf(angle_rad));
        int y2 = g->cy + (int)(outer * sinf(angle_rad));

        g->tick_points[i][0].x = x1;
        g->tick_points[i][0].y = y1;
        g->tick_points[i][1].x = x2;
        g->tick_points[i][1].y = y2;

        g->tick_lines[i] = lv_line_create(g->container);
        lv_obj_set_style_line_width(g->tick_lines[i], width, 0);
        lv_obj_set_style_line_color(g->tick_lines[i], COLOR_ANALOG_DGRAY, 0);
        lv_line_set_points(g->tick_lines[i], g->tick_points[i], 2);
    }

    // Zeiger
    g->needle = lv_line_create(g->container);
    lv_obj_set_style_line_width(g->needle, 3, 0);
    lv_obj_set_style_line_color(g->needle, COLOR_NEEDLE, 0);
    lv_obj_set_style_line_rounded(g->needle, true, 0);
    pts[0].x = g->cx;
    pts[0].y = g->cy;
    pts[1].x = g->cx - g->needle_len;
    pts[1].y = g->cy;
    lv_line_set_points(g->needle, pts, 2);

    // Zeiger Mittelpunkt
    g->needle_center = lv_obj_create(g->container);
    lv_obj_remove_style_all(g->needle_center);
    lv_obj_set_size(g->needle_center, 16, 16);
    lv_obj_set_pos(g->needle_center, g->cx - 8, g->cy - 8);
    lv_obj_set_style_radius(g->needle_center, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(g->needle_center, COLOR_NEEDLE, 0);
    lv_obj_set_style_bg_opa(g->needle_center, LV_OPA_COVER, 0);

    // Value label (Playfair 32 for large values)
    g->value_label = lv_label_create(g->container);
    lv_obj_set_style_text_font(g->value_label, FONT_PLAYFAIR_32, 0);
    lv_obj_set_style_text_color(g->value_label, COLOR_CARD, 0);
    lv_obj_set_style_text_align(g->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(g->value_label, 220);
    lv_obj_set_pos(g->value_label, 0, 125);  // Centered between gauge (cy=110) and name
    lv_label_set_text(g->value_label, "--");

    // Name label (Playfair Serif for elegant look)
    g->name_label = lv_label_create(g->container);
    lv_obj_set_style_text_font(g->name_label, FONT_PLAYFAIR_14, 0);
    lv_obj_set_style_text_color(g->name_label, COLOR_ANALOG_DGRAY, 0);
    lv_obj_set_style_text_align(g->name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(g->name_label, 220);
    lv_obj_set_pos(g->name_label, 0, 162);  // Unter dem Wert
    lv_label_set_text(g->name_label, name);
}

// ------------------------------------------------------------
// Analog Screen erstellen
// ------------------------------------------------------------
static void create_screen3_analog() {
    screens[UI_SCREEN_ANALOG] = lv_obj_create(NULL);
    lv_obj_t* scr = screens[UI_SCREEN_ANALOG];
    
    // Dunkler Hintergrund
    lv_obj_set_style_bg_color(scr, COLOR_ANALOG_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Hauptcontainer mit goldenem Rand
    lv_obj_t* main_cont = lv_obj_create(scr);
    lv_obj_remove_style_all(main_cont);
    lv_obj_set_size(main_cont, 480 - 8, 320 - 8);
    lv_obj_center(main_cont);
    lv_obj_set_style_border_width(main_cont, 4, 0);
    lv_obj_set_style_border_color(main_cont, COLOR_ANALOG_BORDER, 0);
    lv_obj_set_style_radius(main_cont, 12, 0);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);

    // INSPECTAIR Branding
    s3_lbl_brand = lv_label_create(main_cont);
    lv_obj_set_style_text_font(s3_lbl_brand, FONT_12, 0);
    lv_obj_set_style_text_color(s3_lbl_brand, COLOR_ANALOG_BORDER, 0);
    lv_obj_set_style_text_letter_space(s3_lbl_brand, 6, 0);
    lv_label_set_text(s3_lbl_brand, "INSPECTAIR");
    lv_obj_align(s3_lbl_brand, LV_ALIGN_TOP_MID, 0, 8);

    // Uhrzeit (Playfair Display Serif-Font)
    s3_lbl_time = lv_label_create(main_cont);
    lv_obj_set_style_text_font(s3_lbl_time, FONT_PLAYFAIR_48, 0);
    lv_obj_set_style_text_color(s3_lbl_time, COLOR_ANALOG_BEIGE, 0);
    lv_label_set_text(s3_lbl_time, "00:00");
    lv_obj_align(s3_lbl_time, LV_ALIGN_TOP_MID, 0, 32);

    // Datum (Playfair Display Serif-Font)
    s3_lbl_date = lv_label_create(main_cont);
    lv_obj_set_style_text_font(s3_lbl_date, FONT_PLAYFAIR_14, 0);
    lv_obj_set_style_text_color(s3_lbl_date, COLOR_ANALOG_GRAY, 0);
    lv_label_set_text(s3_lbl_date, "Di, 28. Jan");
    lv_obj_align(s3_lbl_date, LV_ALIGN_TOP_MID, 0, 85);

    // Gauges Layout: klein | GROSS | klein
    int gauge_y_small = 178;
    int gauge_y_big = 100;
    int gap = 8;

    // Container-Breite ist (480 - 8) = 472
    int container_w = 480 - 8;
    int total_w = 100 + gap + 220 + gap + 100;  // = 436
    int start_x = (container_w - total_w) / 2;

    // Temperatur (links, klein)
    s3_create_small_gauge(main_cont, &s3_gauge_temp, start_x, gauge_y_small,
                          10.0f, 35.0f, COLOR_TEMP_GAUGE, "°C", "Temperatur", s3_needle_pts_temp);

    // CO2 (center, large)
    s3_create_big_gauge(main_cont, &s3_gauge_co2, start_x + 100 + gap, gauge_y_big,
                        400.0f, 2000.0f, COLOR_CO2_GAUGE, "ppm", "CO2", s3_needle_pts_co2);

    // Humidity (right, small)
    s3_create_small_gauge(main_cont, &s3_gauge_hum, start_x + 100 + gap + 220 + gap, gauge_y_small,
                          0.0f, 100.0f, COLOR_HUM_GAUGE, "%", "Humidity", s3_needle_pts_hum);

    Serial.println("[UI] Screen 3 (analog cockpit) created");
}

// ------------------------------------------------------------
// Analog Screen Update
// ------------------------------------------------------------
static void update_screen3_time() {
    if (!s3_lbl_time) return;
    
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", cached_hour, cached_min);
    lv_label_set_text(s3_lbl_time, buf);
    
    if (s3_lbl_date) {
        lv_label_set_text(s3_lbl_date, cached_date);
    }
}

static void update_screen3_sensors() {
    // Temperatur
    s3_gauge_temp.current_value = cached_temp;
    s3_update_gauge(&s3_gauge_temp, s3_needle_pts_temp);
    
    // CO2
    s3_gauge_co2.current_value = (float)cached_co2;
    s3_update_gauge(&s3_gauge_co2, s3_needle_pts_co2);
    
    // Feuchtigkeit
    s3_gauge_hum.current_value = cached_hum;
    s3_update_gauge(&s3_gauge_hum, s3_needle_pts_hum);
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
 * SCREEN 4: BUBBLE UI (Dynamic Circles)
 * ═══════════════════════════════════════════════════════════════════════════ */

// Colors for bubble screen
#define COLOR_BUBBLE_BG_DARK   lv_color_hex(0x0f0f23)  // Dark blue-black
#define COLOR_BUBBLE_BG_LIGHT  lv_color_hex(0x1a1a3e)  // Dark blue
#define COLOR_BUBBLE_TEXT_DIM  lv_color_hex(0x999999)  // 60% white
#define COLOR_BUBBLE_TEXT_DIMMER lv_color_hex(0x666666)  // 40% white
#define COLOR_BUBBLE_STAR      lv_color_hex(0x6496FF)  // Blue-white stars

// Bubble Layout
#define BUBBLE_MIN_SIZE  95
#define BUBBLE_MAX_SIZE  130

// Bubble Struktur
struct Bubble {
    lv_obj_t* container;
    lv_obj_t* lbl_value;
    lv_obj_t* lbl_unit;
    lv_obj_t* lbl_label;
    int center_x;
    int center_y;
    int current_size;
};

// Screen 4 Elemente
static lv_obj_t* s4_lbl_time = nullptr;
static lv_obj_t* s4_lbl_date = nullptr;
static Bubble s4_bubbles[5];
static lv_style_t style_bubble_base;
static bool style_bubble_initialized = false;

// Calculate bubble size based on value
static int s4_get_bubble_size(int type, float value) {
    float min_val, max_val, good_min, good_max;

    switch(type) {
        case 0: // Temperatur
            min_val = 10; max_val = 35; good_min = 18; good_max = 26;
            break;
        case 1: // Feuchte
            min_val = 20; max_val = 80; good_min = 30; good_max = 60;
            break;
        case 2: // CO2
            min_val = 400; max_val = 2000; good_min = 0; good_max = 800;
            break;
        case 3: // PM2.5
            min_val = 0; max_val = 50; good_min = 0; good_max = 15;
            break;
        case 4: // VOC
            min_val = 0; max_val = 1000; good_min = 0; good_max = 250;
            break;
        default:
            return BUBBLE_MIN_SIZE;
    }

    if (value < min_val) value = min_val;
    if (value > max_val) value = max_val;

    float severity = 0.0f;

    // Severity only outside of good range
    if (value < good_min) {
        severity = (good_min - value) / (good_min - min_val);
    } else if (value > good_max) {
        severity = (value - good_max) / (max_val - good_max);
    }
    // Values in good range → severity = 0 (small green bubble)

    if (severity < 0.0f) severity = 0.0f;
    if (severity > 1.0f) severity = 1.0f;

    // Size offset per sensor type (+ larger, - smaller)
    int size_offset = 0;
    switch(type) {
        case 1: size_offset = -8; break;  // Humidity: smaller
        case 3: size_offset = 8; break;   // PM2.5: larger
        case 4: size_offset = -8; break;  // VOC: smaller
    }

    return (int)(BUBBLE_MIN_SIZE + severity * (BUBBLE_MAX_SIZE - BUBBLE_MIN_SIZE)) + size_offset;
}

// Calculate bubble status (for color)
static Status s4_get_bubble_status(int type, float value) {
    switch(type) {
        case 0: return get_temp_status(value);
        case 1: return get_hum_status(value);
        case 2: return get_co2_status((int)value);
        case 3: return get_pm25_status((int)value);
        case 4: return get_voc_status((int)value);
        default: return GOOD;
    }
}

// Einzelnen Bubble aktualisieren
static void s4_update_bubble(int idx, float value) {
    if (!s4_bubbles[idx].container) return;
    
    Status status = s4_get_bubble_status(idx, value);
    int new_size = s4_get_bubble_size(idx, value);

    // Position neu berechnen (Mittelpunkt bleibt gleich)
    int x = s4_bubbles[idx].center_x - new_size / 2;
    int y = s4_bubbles[idx].center_y - new_size / 2;

    lv_obj_set_size(s4_bubbles[idx].container, new_size, new_size);
    lv_obj_set_pos(s4_bubbles[idx].container, x, y);

    // Farbe basierend auf Status
    lv_color_t status_color = get_status_color(status);
    lv_obj_set_style_bg_color(s4_bubbles[idx].container, status_color, 0);
    lv_obj_set_style_border_color(s4_bubbles[idx].container, status_color, 0);
    lv_obj_set_style_shadow_color(s4_bubbles[idx].container, status_color, 0);

    // Wert aktualisieren
    char buf[16];
    if (idx == 0) {
        snprintf(buf, sizeof(buf), "%.1f", value);
    } else {
        snprintf(buf, sizeof(buf), "%d", (int)value);
    }
    lv_label_set_text(s4_bubbles[idx].lbl_value, buf);
    lv_obj_set_style_text_color(s4_bubbles[idx].lbl_value, status_color, 0);

    // Elemente neu zentrieren
    lv_obj_align(s4_bubbles[idx].lbl_value, LV_ALIGN_CENTER, 0, -8);
    lv_obj_align(s4_bubbles[idx].lbl_unit, LV_ALIGN_CENTER, 0, 12);
    lv_obj_align(s4_bubbles[idx].lbl_label, LV_ALIGN_CENTER, 0, 28);

    s4_bubbles[idx].current_size = new_size;
}

// Sterne-Positionen (statisch)
static const struct { int x; int y; int size; int opa; } s4_star_data[11] = {
    {50,  45,  2, 180}, {420, 80,  3, 220}, {30,  180, 2, 150},
    {460, 200, 2, 200}, {90,  280, 2, 160}, {380, 290, 3, 190},
    {240, 100, 2, 170}, {240, 180, 2, 200}, {180, 145, 2, 140},
    {300, 145, 2, 150}, {240, 260, 2, 160},
};

// Hintergrund mit Gradient und Sternen erstellen
static void s4_create_background(lv_obj_t* parent) {
    // Gradient Hintergrund
    lv_obj_t* bg = lv_obj_create(parent);
    lv_obj_remove_style_all(bg);
    lv_obj_set_size(bg, 480, 320);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(bg, COLOR_BUBBLE_BG_DARK, 0);
    lv_obj_set_style_bg_grad_color(bg, COLOR_BUBBLE_BG_LIGHT, 0);
    lv_obj_set_style_bg_grad_dir(bg, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    lv_obj_move_to_index(bg, 0);

    // Sterne
    for (int i = 0; i < 11; i++) {
        lv_obj_t* star = lv_obj_create(parent);
        lv_obj_remove_style_all(star);
        lv_obj_set_size(star, s4_star_data[i].size, s4_star_data[i].size);
        lv_obj_set_pos(star, s4_star_data[i].x, s4_star_data[i].y);
        lv_obj_set_style_radius(star, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(star, COLOR_BUBBLE_STAR, 0);
        lv_obj_set_style_bg_opa(star, s4_star_data[i].opa, 0);
        lv_obj_set_style_border_width(star, 0, 0);
        lv_obj_clear_flag(star, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(star, LV_OBJ_FLAG_CLICKABLE);
    }
}

// Legende erstellen
static void s4_create_legend(lv_obj_t* parent) {
    const char* labels[] = {"Gut", "Mittel", "Schlecht"};
    lv_color_t colors[] = {COLOR_GOOD, COLOR_WARN, COLOR_BAD};
    int x_positions[] = {480 - 235, 480 - 165, 480 - 80};

    for (int i = 0; i < 3; i++) {
        // Farbiger Punkt
        lv_obj_t* dot = lv_obj_create(parent);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_pos(dot, x_positions[i], 298 + 2);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, colors[i], 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

        // Text
        lv_obj_t* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_font(lbl, FONT_12, 0);
        lv_obj_set_style_text_color(lbl, COLOR_BUBBLE_TEXT_DIM, 0);
        lv_obj_set_pos(lbl, x_positions[i] + 12, 298);
    }
}

// Bubble Screen erstellen
static void create_screen4_bubble() {
    screens[UI_SCREEN_BUBBLE] = lv_obj_create(NULL);
    lv_obj_t* scr = screens[UI_SCREEN_BUBBLE];

    // Bubble Style initialisieren
    if (!style_bubble_initialized) {
        style_bubble_initialized = true;
        lv_style_init(&style_bubble_base);
        lv_style_set_bg_opa(&style_bubble_base, LV_OPA_20);
        lv_style_set_radius(&style_bubble_base, LV_RADIUS_CIRCLE);
        lv_style_set_border_width(&style_bubble_base, 3);
        lv_style_set_border_opa(&style_bubble_base, LV_OPA_COVER);
        lv_style_set_shadow_width(&style_bubble_base, 30);
        lv_style_set_shadow_opa(&style_bubble_base, LV_OPA_40);
        lv_style_set_shadow_spread(&style_bubble_base, 2);
    }

    s4_create_background(scr);

    // Uhrzeit (oben mittig, Orbitron Space-Font)
    s4_lbl_time = lv_label_create(scr);
    lv_obj_set_style_text_font(s4_lbl_time, FONT_ORBITRON_28, 0);
    lv_obj_set_style_text_color(s4_lbl_time, COLOR_CARD, 0);
    lv_obj_set_style_text_letter_space(s4_lbl_time, 3, 0);
    lv_label_set_text(s4_lbl_time, "00:00");
    lv_obj_align(s4_lbl_time, LV_ALIGN_TOP_MID, 0, 22);

    // Datum (unter Uhrzeit, Orbitron)
    s4_lbl_date = lv_label_create(scr);
    lv_obj_set_style_text_font(s4_lbl_date, FONT_ORBITRON_16, 0);
    lv_obj_set_style_text_color(s4_lbl_date, COLOR_BUBBLE_TEXT_DIM, 0);
    lv_obj_set_style_text_letter_space(s4_lbl_date, 1, 0);
    lv_label_set_text(s4_lbl_date, "Di, 28. Jan 2026");
    lv_obj_align_to(s4_lbl_date, s4_lbl_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    // Bubble positions (asymmetric, well distributed across 480x320)
    // Display: X=0-480, Y available approx. 60-280 (header on top, legend on bottom)
    // Max bubble 110px → centers at least 55px from edge
    const char* labels[] = {"Temp", "Humidity", "CO2", "PM2.5", "VOC"};
    const char* units[] = {"C", "%", "ppm", "ug/m3", "ppb"};
    //                      Temp   Feuchte  CO2    PM2.5   VOC
    int center_x[] = {      85,    395,     255,   110,    355};
    int center_y[] = {      100,   110,     160,   245,    255};

    for (int i = 0; i < 5; i++) {
        s4_bubbles[i].center_x = center_x[i];
        s4_bubbles[i].center_y = center_y[i];
        s4_bubbles[i].current_size = BUBBLE_MIN_SIZE;

        int initial_size = BUBBLE_MIN_SIZE;
        int x = center_x[i] - initial_size / 2;
        int y = center_y[i] - initial_size / 2;

        // Kreis-Container
        s4_bubbles[i].container = lv_obj_create(scr);
        lv_obj_remove_style_all(s4_bubbles[i].container);
        lv_obj_set_size(s4_bubbles[i].container, initial_size, initial_size);
        lv_obj_set_pos(s4_bubbles[i].container, x, y);
        lv_obj_add_style(s4_bubbles[i].container, &style_bubble_base, 0);
        lv_obj_set_style_bg_color(s4_bubbles[i].container, COLOR_GOOD, 0);
        lv_obj_set_style_border_color(s4_bubbles[i].container, COLOR_GOOD, 0);
        lv_obj_set_style_shadow_color(s4_bubbles[i].container, COLOR_GOOD, 0);
        lv_obj_clear_flag(s4_bubbles[i].container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(s4_bubbles[i].container, 0, 0);

        // Wert Label
        s4_bubbles[i].lbl_value = lv_label_create(s4_bubbles[i].container);
        lv_obj_set_style_text_font(s4_bubbles[i].lbl_value, FONT_28, 0);
        lv_obj_set_style_text_color(s4_bubbles[i].lbl_value, COLOR_GOOD, 0);
        lv_label_set_text(s4_bubbles[i].lbl_value, "--");
        lv_obj_center(s4_bubbles[i].lbl_value);

        // Einheit Label
        s4_bubbles[i].lbl_unit = lv_label_create(s4_bubbles[i].container);
        lv_obj_set_style_text_font(s4_bubbles[i].lbl_unit, FONT_12, 0);
        lv_obj_set_style_text_color(s4_bubbles[i].lbl_unit, COLOR_BUBBLE_TEXT_DIM, 0);
        lv_label_set_text(s4_bubbles[i].lbl_unit, units[i]);
        lv_obj_align_to(s4_bubbles[i].lbl_unit, s4_bubbles[i].lbl_value, LV_ALIGN_OUT_BOTTOM_MID, 0, -2);

        // Beschriftung
        s4_bubbles[i].lbl_label = lv_label_create(s4_bubbles[i].container);
        lv_obj_set_style_text_font(s4_bubbles[i].lbl_label, FONT_12, 0);
        lv_obj_set_style_text_color(s4_bubbles[i].lbl_label, COLOR_BUBBLE_TEXT_DIMMER, 0);
        lv_label_set_text(s4_bubbles[i].lbl_label, labels[i]);
        lv_obj_align_to(s4_bubbles[i].lbl_label, s4_bubbles[i].lbl_unit, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    }

    // No legend - bubbles are self-explanatory

    Serial.println("[UI] Screen 4 (bubble) created");
}

// Bubble Screen Zeit aktualisieren
static void update_screen4_time() {
    if (!s4_lbl_time) return;
    
    char buf[12];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", cached_hour, cached_min, cached_sec);
    lv_label_set_text(s4_lbl_time, buf);
    
    if (s4_lbl_date) {
        lv_label_set_text(s4_lbl_date, cached_date);
    }
}

// Bubble Screen Sensoren aktualisieren
static void update_screen4_sensors() {
    s4_update_bubble(0, cached_temp);
    s4_update_bubble(1, cached_hum);
    s4_update_bubble(2, (float)cached_co2);
    s4_update_bubble(3, (float)cached_pm25);
    s4_update_bubble(4, (float)cached_voc);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC API FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

void ui_init() {
    Serial.println("[UI] Initializing multi-screen UI...");
    
    init_styles();
    
    // Create all five screens
    create_screen0_tree();   // Tree animation (start screen)
    create_screen1();        // Overview (minimalistic)
    create_screen2();        // Detail (full info)
    create_screen3_analog(); // Analog cockpit (instruments)
    create_screen4_bubble(); // Dynamic circles (bubbles)
    
    // Start with screen 0 (tree animation)
    current_screen = UI_SCREEN_TREE;
    lv_screen_load(screens[current_screen]);
    
    Serial.println("[UI] Multi-screen UI initialized, starting with tree animation");
}

void ui_nextScreen() {
    Serial.println("[UI] ui_nextScreen() called");
    Serial.flush();
    Serial.printf("[UI] Current screen: %d, screens[0]=%p, screens[1]=%p\n", 
                  current_screen, screens[0], screens[1]);
    Serial.flush();
    
    int next = (current_screen + 1) % UI_SCREEN_COUNT;
    Serial.printf("[UI] Switching to screen: %d\n", next);
    Serial.flush();
    ui_setScreen((UIScreen)next);
}

void ui_setScreen(UIScreen screen) {
    Serial.printf("[UI] ui_setScreen(%d) called\n", screen);
    Serial.flush();
    
    if (screen >= UI_SCREEN_COUNT) {
        Serial.println("[UI] ERROR: Screen index invalid!");
        return;
    }
    if (screens[screen] == nullptr) {
        Serial.println("[UI] ERROR: Screen is NULL!");
        return;
    }
    
    Serial.println("[UI] Starting screen transition...");
    Serial.flush();
    current_screen = screen;
    
    // Simple load instead of animation (safer)
    Serial.println("[UI] Before lv_screen_load()...");
    Serial.flush();
    lv_screen_load(screens[screen]);
    Serial.println("[UI] After lv_screen_load()");
    Serial.flush();
    
    // Aktuellen Screen mit gecachten Werten aktualisieren
    if (screen == UI_SCREEN_TREE) {
        Serial.println("[UI] Vor update_screen0_tree()...");
        Serial.flush();
        update_screen0_tree();
    } else if (screen == UI_SCREEN_OVERVIEW) {
        Serial.println("[UI] Vor update_screen1_time()...");
        Serial.flush();
        update_screen1_time();
        Serial.println("[UI] Vor update_screen1_sensors()...");
        Serial.flush();
        update_screen1_sensors();
    } else if (screen == UI_SCREEN_DETAIL) {
        Serial.println("[UI] Vor update_screen2_time()...");
        Serial.flush();
        update_screen2_time();
        Serial.println("[UI] Vor update_screen2_sensors()...");
        Serial.flush();
        update_screen2_sensors();
    } else if (screen == UI_SCREEN_ANALOG) {
        Serial.println("[UI] Vor update_screen3_time()...");
        Serial.flush();
        update_screen3_time();
        Serial.println("[UI] Vor update_screen3_sensors()...");
        Serial.flush();
        update_screen3_sensors();
    } else if (screen == UI_SCREEN_BUBBLE) {
        Serial.println("[UI] Vor update_screen4_time()...");
        Serial.flush();
        update_screen4_time();
        Serial.println("[UI] Vor update_screen4_sensors()...");
        Serial.flush();
        update_screen4_sensors();
    }
    
    const char* screen_names[] = {"Tree Animation", "Overview", "Detail", "Analog Cockpit", "Bubbles"};
    Serial.printf("[UI] Switch to screen %d (%s) completed\n", screen, screen_names[screen]);
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
    
    // Alle Screens aktualisieren (nur der aktive ist sichtbar)
    update_screen1_time();
    update_screen2_time();
    update_screen3_time();
    update_screen4_time();
}

void ui_updateDate(const char* date_str) {
    if (!date_str) return;
    strncpy(cached_date, date_str, sizeof(cached_date) - 1);
    cached_date[sizeof(cached_date) - 1] = '\0';
    
    if (s1_lbl_date) lv_label_set_text(s1_lbl_date, cached_date);
    if (s2_lbl_date) lv_label_set_text(s2_lbl_date, cached_date);
    if (s3_lbl_date) lv_label_set_text(s3_lbl_date, cached_date);
    if (s4_lbl_date) lv_label_set_text(s4_lbl_date, cached_date);
}

void ui_updateSensorValues(float temp, float hum, int co2, int pm25, int voc) {
    // Cache aktualisieren
    cached_temp = temp;
    cached_hum = hum;
    cached_co2 = co2;
    cached_pm25 = pm25;
    cached_voc = voc;
    
    Serial.printf("[UI] Update: T=%.1f H=%.0f CO2=%d PM=%d VOC=%d\n", temp, hum, co2, pm25, voc);
    
    // Alle Screens aktualisieren
    update_screen0_tree();    // Tree screen changes color based on air quality
    update_screen1_sensors();
    update_screen2_sensors();
    update_screen3_sensors(); // Analog Cockpit
    update_screen4_sensors(); // Bubbles
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
