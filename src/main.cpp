#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// Projekt-Header
#include "pins.h"
#include "colors.h"
#include "display_config.h"
#include "sensor_types.h"
#include "display/ui_manager.h"
#include "sensors/aht_sgp.h"
#include "sensors/mhz19c.h"
#include "sensors/pms5003.h"
#include "sensors/ld2410c.h"
#include "WifiClock.h"

// ============================================
// INSPECTAIR - LUFTQUALITÄTSMESSGERÄT
// Modulare Architektur v2.0
// ============================================

LGFX tft;
WifiClock myClock;

void setup() {
  // Backlight als Output
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  delay(1000);
  // Serial.end(); // ENTFERNT: Das hat die USB-Verbindung gekappt!
  Serial.begin(115200); // Wieder an, damit wir Fehler sehen
  delay(2000);

  Serial.println("\n\n=== INSPECTAIR START ===");
  Serial.println("[INIT] System initialisiert sich...\n");

  // WLAN und Uhrzeit starten
  myClock.begin("Pommes mit Mayo", "160506060406$$$");

  // === DISPLAY INITIALISIEREN ===
  Serial.println("[INIT] Display...");
  ui_init(); // Sofort initialisieren, damit man was sieht!

  // === I2C SENSOREN ===
  Serial.println("[INIT] I2C-Sensoren (AHT20, SGP40)...");
  if (!sensors_i2c_init()) {
    Serial.println("[ERROR] I2C-Sensoren konnten nicht initialisiert werden!");
  }

  // === UART SENSOREN ===
  Serial.println("[INIT] UART-Sensoren...");

  if (!sensors_pms_init()) {
    Serial.println("[ERROR] PMS5003 konnte nicht initialisiert werden!");
  }
  
  if (!sensors_mhz19_init()) {
    Serial.println("[ERROR] MH-Z19C konnte nicht initialisiert werden!");
  }

  if (!sensors_radar_init()) {
    Serial.println("[ERROR] LD2410C konnte nicht initialisiert werden!");
  }

  Serial.println("\n[INFO] Alle Sensoren initialisiert!");
  Serial.println("[INFO] Display-Update alle 2 Sekunden\n");

}

void loop() {
  static unsigned long lastUpdate = 0;
  static SensorReadings readings = {0};
  static unsigned long lastTimeUpdate = 0;
  static unsigned long last_pms_ok = 0;
  static unsigned long last_radar_ok = 0;

  // === KONTINUIERLICHE SENSOR-LESEVORGÄNGE ===
  // PMS5003 kontinuierlich auslesen (asynchron)
  if (sensors_pms_read(&readings.pms)) {
    last_pms_ok = millis();
  }

  // Radar kontinuierlich auslesen (WICHTIG für Buffer!)
  if (sensors_radar_read(&readings.radar)) {
    last_radar_ok = millis();
  }

  // === UHRZEIT UPDATE (jede Sekunde) ===
  if (millis() - lastTimeUpdate > 1000) {
    lastTimeUpdate = millis();
    String currentTime = myClock.getFormattedTime();

    // Uhrzeit oben rechts anzeigen (überschreibt alten Wert durch Hintergrundfarbe)
    tft.setTextSize(2);
    tft.setTextColor(COLOR_TITLE, COLOR_BG); 
    tft.setTextDatum(TC_DATUM);
    tft.drawString(currentTime, 240, 8); // Zentriert oben (480/2 = 240)
  }

  // === PERIODISCHE UPDATES (alle 2 Sekunden) ===
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();

    // Aktuelle Zeit speichern
    readings.timestamp = millis();

    // === SENSOR AUSLESEN ===
    bool aht_ok = sensors_aht20_read(&readings.aht);
    bool mhz_ok = sensors_mhz19_read(&readings.mhz);
    
    // Prüfen, ob in den letzten 5 Sekunden Daten kamen
    bool pms_ok = (millis() - last_pms_ok < 5000);
    bool radar_ok = (millis() - last_radar_ok < 5000);

    // SGP40 benötigt aktuelle Temp/Humidity
    if (aht_ok) {
      sensors_sgp40_read(readings.aht.temperature,
                         readings.aht.humidity,
                         &readings.sgp);
    }

    // === DISPLAY UPDATE ===
    // Immer aktualisieren, verhindert Glitchen bei kurzen Sensor-Aussetzern
    ui_updateDisplay(readings);

    // === SERIAL AUSGABE (für Debugging) ===
    Serial.printf("[%lu] T:%.1f H:%.0f CO2:%ld VOC:%ld PM2.5:%u PM10:%u Radar:%d\n",
                  readings.timestamp,
                  readings.aht.temperature,
                  readings.aht.humidity,
                  readings.mhz.co2_ppm,
                  readings.sgp.voc_index,
                  readings.pms.PM_AE_UG_2_5,
                  readings.pms.PM_AE_UG_10_0,
                  readings.radar.presence);
  }
}
