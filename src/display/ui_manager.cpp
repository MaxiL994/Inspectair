#include "ui_manager.h"
#include "pins.h"
#include "colors.h"
#include "display_config.h"

extern LGFX tft;

// Layout Konstanten
#define BOX_MARGIN    10   // Erhöht, damit links nichts abgeschnitten wird
#define BOX_PADDING   10   // Mehr Innenabstand
#define ROW_HEIGHT    90
#define COL_WIDTH     225  // Etwas schmaler, damit es mit größerem Margin passt (10+225+10+225+10 = 480)

// Sprite für flackerfreies Zeichnen der Werte
static LGFX_Sprite* valSprite = nullptr;

// Hilfsfunktion: Zeichnet eine statische Box (Hintergrund + Labels)
// Wird nur EINMAL aufgerufen
void ui_drawBoxStatic(int x, int y, int w, int h, const char* label, const char* unit) {
  tft.fillRoundRect(x, y, w, h, 8, COLOR_BOX_BG);
  tft.drawRoundRect(x, y, w, h, 8, 0x4208); // Dunkler Rahmen

  // Label oben links
  tft.setFont(nullptr);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_LABEL);
  tft.setTextSize(1);
  tft.drawString(label, x + BOX_PADDING, y + BOX_PADDING);

  // Einheit unten links
  tft.setFont(nullptr);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(COLOR_LABEL);
  tft.setTextSize(1);
  tft.drawString(unit, x + BOX_PADDING, y + h - BOX_PADDING - 2); // 2px höher
}

// Hilfsfunktion: Aktualisiert nur den Wert (mit Sprite gegen Flackern)
void ui_drawValue(int x, int y, int w, int h, const char* value, uint16_t color) {
  if (!valSprite) return;
  
  // Sprite löschen (mit Box-Hintergrundfarbe füllen)
  valSprite->fillScreen(COLOR_BOX_BG);
  
  // Wert in das Sprite zeichnen
  valSprite->setFont(&FreeSansBold18pt7b);
  valSprite->setTextColor(color);
  valSprite->setTextDatum(MC_DATUM);
  valSprite->drawString(value, valSprite->width() / 2, valSprite->height() / 2);
  
  // Das fertige Sprite auf das Display kopieren (zentriert in der Box)
  // Vertikal zentriert in der Box, um Platz für Label und Einheit zu lassen
  valSprite->pushSprite(x + BOX_PADDING, y + (h - valSprite->height()) / 2);
}

void ui_init() {
  // Display Hardware Init
  tft.init();
  tft.setRotation(1);
  
  // Sprite erstellen (Breite = Boxbreite - Padding, Höhe = 40px für die Schrift)
  if (valSprite == nullptr) {
    valSprite = new LGFX_Sprite(&tft);
    valSprite->createSprite(COL_WIDTH - (BOX_PADDING * 2), 30);
  }

  // Hintergrund und Titel
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_TITLE);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("INSPECTAIR");

  // Statisches Layout zeichnen (alle Boxen leer)
  int row1 = 35;
  int row2 = row1 + ROW_HEIGHT + BOX_MARGIN;
  int row3 = row2 + ROW_HEIGHT + BOX_MARGIN;
  int col1 = BOX_MARGIN;
  int col2 = col1 + COL_WIDTH + BOX_MARGIN;

  ui_drawBoxStatic(col1, row1, COL_WIDTH, ROW_HEIGHT, "TEMPERATUR", "Grad Celsius");
  ui_drawBoxStatic(col2, row1, COL_WIDTH, ROW_HEIGHT, "LUFTFEUCHTE", "% rel. Feuchte");
  
  ui_drawBoxStatic(col1, row2, COL_WIDTH, ROW_HEIGHT, "CO2", "ppm");
  ui_drawBoxStatic(col2, row2, COL_WIDTH, ROW_HEIGHT, "VOC INDEX", "1-500");
  
  ui_drawBoxStatic(col1, row3, COL_WIDTH, ROW_HEIGHT, "FEINSTAUB PM2.5", "ug/m3");
  ui_drawBoxStatic(col2, row3, COL_WIDTH, ROW_HEIGHT, "FEINSTAUB PM10", "ug/m3");
}

void ui_updateDisplay(const SensorReadings& readings) {
  static SensorReadings last = {0};
  static bool first = true;
  
  int row1 = 35;
  int row2 = row1 + ROW_HEIGHT + BOX_MARGIN;
  int row3 = row2 + ROW_HEIGHT + BOX_MARGIN;
  int col1 = BOX_MARGIN;
  int col2 = col1 + COL_WIDTH + BOX_MARGIN;
  
  char buf[16];

  // Nur aktualisieren, wenn sich der Wert geändert hat (oder beim ersten Mal)

  // 1. Temperatur (Änderung > 0.1)
  if (first || abs(readings.aht.temperature - last.aht.temperature) > 0.1) {
    sprintf(buf, "%.1f", readings.aht.temperature);
    ui_drawValue(col1, row1, COL_WIDTH, ROW_HEIGHT, buf, getTempColor(readings.aht.temperature));
  }

  // 2. Luftfeuchte (Änderung > 0.9)
  if (first || abs(readings.aht.humidity - last.aht.humidity) > 0.9) {
    sprintf(buf, "%.0f", readings.aht.humidity);
    ui_drawValue(col2, row1, COL_WIDTH, ROW_HEIGHT, buf, getHumColor(readings.aht.humidity));
  }

  // 3. CO2
  if (first || readings.mhz.co2_ppm != last.mhz.co2_ppm) {
    if (readings.mhz.co2_ppm <= 0) {
       ui_drawValue(col1, row2, COL_WIDTH, ROW_HEIGHT, "WAIT", COLOR_LABEL);
    } else {
       sprintf(buf, "%d", readings.mhz.co2_ppm);
       ui_drawValue(col1, row2, COL_WIDTH, ROW_HEIGHT, buf, getCO2Color(readings.mhz.co2_ppm));
    }
  }

  // 4. VOC
  if (first || readings.sgp.voc_index != last.sgp.voc_index) {
    if (readings.sgp.voc_index <= 0) {
       ui_drawValue(col2, row2, COL_WIDTH, ROW_HEIGHT, "INIT", COLOR_LABEL);
    } else {
       sprintf(buf, "%d", readings.sgp.voc_index);
       ui_drawValue(col2, row2, COL_WIDTH, ROW_HEIGHT, buf, getVOCColor(readings.sgp.voc_index));
    }
  }

  // 5. PM2.5
  if (first || readings.pms.PM_AE_UG_2_5 != last.pms.PM_AE_UG_2_5) {
    sprintf(buf, "%u", readings.pms.PM_AE_UG_2_5);
    ui_drawValue(col1, row3, COL_WIDTH, ROW_HEIGHT, buf, getPM25Color(readings.pms.PM_AE_UG_2_5));
  }

  // 6. PM10
  if (first || readings.pms.PM_AE_UG_10_0 != last.pms.PM_AE_UG_10_0) {
    sprintf(buf, "%u", readings.pms.PM_AE_UG_10_0);
    ui_drawValue(col2, row3, COL_WIDTH, ROW_HEIGHT, buf, getPM25Color(readings.pms.PM_AE_UG_10_0));
  }

  last = readings;
  first = false;
}

void ui_showError(const char* message) {
  // Optional: Fehleranzeige
}