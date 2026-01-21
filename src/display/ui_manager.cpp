#include "ui_manager.h"
#include "../include/colors.h"
#include "../include/pins.h"
#include <stdio.h>

// ============================================
// UI MANAGER IMPLEMENTATION
// ============================================

LGFX tft;

void ui_init(void) {
  // Backlight anschalten
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);
  
  delay(1000);
  
  // Display initialisieren
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);
  tft.setTextDatum(TL_DATUM);
  
  // Startbildschirm
  tft.setTextColor(COLOR_TEXT);
  tft.setTextSize(3);
  tft.setCursor(100, 120);
  tft.println("INSPECTAIR");
  tft.setTextSize(1);
  tft.setCursor(130, 170);
  tft.println("Initialisiere Sensoren...");
  
  // Hauptanzeige vorbereiten
  delay(1000);
  tft.fillScreen(COLOR_BG);
  
  // Titel
  tft.setTextColor(COLOR_TITLE);  // Cyan
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("INSPECTAIR");
}

void ui_drawSensorBox(int x, int y, int w, int h, 
                      const char* label, const char* value, 
                      const char* unit, uint16_t valueColor) {
  tft.fillRoundRect(x, y, w, h, 8, COLOR_BOX_BG);
  tft.drawRoundRect(x, y, w, h, 8, 0x4208);

  // Label oben
  tft.setFont(nullptr);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COLOR_LABEL);
  tft.setTextSize(1);
  tft.drawString(label, x + BOX_PADDING, y + BOX_PADDING);

  // Wert gross in der Mitte - hochauflösende Font
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(valueColor);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.drawString(value, x + w / 2, y + h / 2 + 8);

  // Einheit unten
  tft.setFont(nullptr);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(COLOR_LABEL);
  tft.setTextSize(1);
  tft.drawString(unit, x + BOX_PADDING, y + h - BOX_PADDING);
}

void ui_updateDisplay(const SensorReadings& readings) {
  // Layout: 2 Spalten, 3 Reihen
  int row1_y = 38;
  int row2_y = row1_y + ROW_HEIGHT + BOX_MARGIN;
  int row3_y = row2_y + ROW_HEIGHT + BOX_MARGIN;
  int col1_x = BOX_MARGIN;
  int col2_x = col1_x + COL_WIDTH + BOX_MARGIN;

  // Strings vorbereiten
  char tempStr[10], humStr[10], co2Str[10], vocStr[10], pm25Str[10], pm10Str[10];
  sprintf(tempStr, "%.1f", readings.aht.temperature);
  sprintf(humStr, "%.0f", readings.aht.humidity);
  sprintf(co2Str, "%ld", readings.mhz.co2_ppm);
  sprintf(vocStr, "%ld", readings.sgp.voc_index);
  sprintf(pm25Str, "%u", readings.pms.PM_AE_UG_2_5);
  sprintf(pm10Str, "%u", readings.pms.PM_AE_UG_10_0);

  // Reihe 1: Temperatur & Luftfeuchtigkeit
  ui_drawSensorBox(col1_x, row1_y, COL_WIDTH, ROW_HEIGHT,
                   "TEMPERATUR", tempStr, "°C", 
                   getTempColor(readings.aht.temperature));
  ui_drawSensorBox(col2_x, row1_y, COL_WIDTH, ROW_HEIGHT,
                   "LUFTFEUCHTE", humStr, "% rel.Feucht.", 
                   getHumColor(readings.aht.humidity));

  // Reihe 2: CO2 & VOC
  ui_drawSensorBox(col1_x, row2_y, COL_WIDTH, ROW_HEIGHT,
                   "CO2", co2Str, "ppm", 
                   getCO2Color(readings.mhz.co2_ppm));
  ui_drawSensorBox(col2_x, row2_y, COL_WIDTH, ROW_HEIGHT,
                   "VOC INDEX", vocStr, "1-500", 
                   getVOCColor(readings.sgp.voc_index));

  // Reihe 3: PM2.5 & PM10
  ui_drawSensorBox(col1_x, row3_y, COL_WIDTH, ROW_HEIGHT,
                   "FEINSTAUB PM2.5", pm25Str, "µg/m³", 
                   getPM25Color(readings.pms.PM_AE_UG_2_5));
  ui_drawSensorBox(col2_x, row3_y, COL_WIDTH, ROW_HEIGHT,
                   "FEINSTAUB PM10", pm10Str, "µg/m³", 
                   getPM25Color(readings.pms.PM_AE_UG_10_0));
}

void ui_showError(const char* message) {
  tft.fillScreen(COLOR_BG);
  tft.setTextColor(COLOR_DANGER);
  tft.setTextDatum(MC_DATUM);
  tft.setCursor(160, 240);
  tft.println(message);
}

void ui_setBacklight(uint8_t brightness) {
  // PWM für Backlight-Dimmung
  analogWrite(PIN_TFT_BL, brightness);
}
