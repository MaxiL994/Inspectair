#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP40.h>
#include <PMS.h>
#include <MHZ19.h>
#include <ld2410.h>

// Display Konfiguration fuer ST7796S
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 10000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 12;
      cfg.pin_mosi = 11;
      cfg.pin_miso = -1;
      cfg.pin_dc = 9;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 14;
      cfg.pin_rst = 46;
      cfg.pin_busy = -1;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

// PIN DEFINITIONEN
#define PIN_I2C_SDA  8
#define PIN_I2C_SCL  18
#define PIN_PMS_RX   16
#define PIN_PMS_TX   17
#define PIN_CO2_RX   4
#define PIN_CO2_TX   5
#define PIN_RADAR_RX 6
#define PIN_RADAR_TX 7
#define PIN_TFT_BL   3

// UART Objekte
HardwareSerial SerialPMS(1);
HardwareSerial SerialCO2(2);

// Sensor Objekte
Adafruit_AHTX0 aht;
Adafruit_SGP40 sgp;
PMS pms(SerialPMS);
PMS::DATA pmsData;
MHZ19 myMHZ19;

// Display Objekt
LGFX tft;

// Farben (RGB565)
#define COLOR_GOOD     0x07E0  // Gruen
#define COLOR_MODERATE 0xFFE0  // Gelb
#define COLOR_BAD      0xFD20  // Orange
#define COLOR_DANGER   0xF800  // Rot
#define COLOR_BG       0x0000  // Schwarz
#define COLOR_TEXT     0xFFFF  // Weiss
#define COLOR_LABEL    0xAD55  // Grau
#define COLOR_BOX_BG   0x1082  // Dunkles Blaugrau

// Farbfunktionen
uint16_t getCO2Color(int co2) {
  if (co2 < 800) return COLOR_GOOD;
  if (co2 < 1000) return COLOR_MODERATE;
  if (co2 < 1500) return COLOR_BAD;
  return COLOR_DANGER;
}

uint16_t getPM25Color(int pm25) {
  if (pm25 < 12) return COLOR_GOOD;
  if (pm25 < 35) return COLOR_MODERATE;
  if (pm25 < 55) return COLOR_BAD;
  return COLOR_DANGER;
}

uint16_t getVOCColor(int voc) {
  if (voc < 100) return COLOR_GOOD;
  if (voc < 200) return COLOR_MODERATE;
  if (voc < 300) return COLOR_BAD;
  return COLOR_DANGER;
}

uint16_t getTempColor(float temp) {
  if (temp >= 18 && temp <= 24) return COLOR_GOOD;
  if (temp >= 15 && temp <= 28) return COLOR_MODERATE;
  return COLOR_BAD;
}

uint16_t getHumColor(float hum) {
  if (hum >= 40 && hum <= 60) return COLOR_GOOD;
  if (hum >= 30 && hum <= 70) return COLOR_MODERATE;
  return COLOR_BAD;
}

// Display Layout
#define BOX_MARGIN    6
#define BOX_PADDING   8
#define ROW_HEIGHT    90
#define COL_WIDTH     228

void drawSensorBox(int x, int y, int w, int h, const char* label, const char* value, const char* unit, uint16_t valueColor) {
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

void setup() {
  // Backlight an
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  delay(1000);
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n=== INSPECTAIR START ===");

  // Display init
  Serial.println("[INIT] Display...");
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

  // I2C Sensoren
  Serial.println("[INIT] I2C...");
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(500);

  bool ahtOK = aht.begin();
  Serial.printf("  AHT20: %s\n", ahtOK ? "OK" : "FEHLER");

  bool sgpOK = sgp.begin();
  Serial.printf("  SGP40: %s\n", sgpOK ? "OK" : "FEHLER");

  // UART Sensoren
  Serial.println("[INIT] UART...");

  SerialPMS.begin(9600, SERIAL_8N1, PIN_PMS_RX, PIN_PMS_TX);
  Serial.println("  PMS5003: OK");
  delay(500);

  SerialCO2.begin(9600, SERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX);
  delay(1000);
  myMHZ19.begin(SerialCO2);
  myMHZ19.autoCalibration(false);
  Serial.println("  MH-Z19C: OK");

  Serial.println("[INIT] Fertig!");

  // Hauptanzeige vorbereiten
  delay(1000);
  tft.fillScreen(COLOR_BG);

  // Titel
  tft.setTextColor(0x07FF);  // Cyan
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("INSPECTAIR");
}

void loop() {
  static unsigned long lastUpdate = 0;

  // PMS kontinuierlich lesen
  pms.read(pmsData);

  // Alle 2 Sekunden Update
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();

    // Sensordaten auslesen
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    int32_t vocIndex = sgp.measureVocIndex(temp.temperature, humidity.relative_humidity);
    int co2 = myMHZ19.getCO2();

    // Strings vorbereiten
    char tempStr[10], humStr[10], co2Str[10], vocStr[10], pm25Str[10], pm10Str[10];
    sprintf(tempStr, "%.1f", temp.temperature);
    sprintf(humStr, "%.0f", humidity.relative_humidity);
    sprintf(co2Str, "%d", co2);
    sprintf(vocStr, "%d", vocIndex);
    sprintf(pm25Str, "%d", pmsData.PM_AE_UG_2_5);
    sprintf(pm10Str, "%d", pmsData.PM_AE_UG_10_0);

    // Layout: 2 Spalten, 3 Reihen
    int row1_y = 38;
    int row2_y = row1_y + ROW_HEIGHT + BOX_MARGIN;
    int row3_y = row2_y + ROW_HEIGHT + BOX_MARGIN;
    int col1_x = BOX_MARGIN;
    int col2_x = col1_x + COL_WIDTH + BOX_MARGIN;

    // Reihe 1: Temperatur & Luftfeuchtigkeit
    drawSensorBox(col1_x, row1_y, COL_WIDTH, ROW_HEIGHT,
                  "TEMPERATUR", tempStr, "Grad Celsius", getTempColor(temp.temperature));
    drawSensorBox(col2_x, row1_y, COL_WIDTH, ROW_HEIGHT,
                  "LUFTFEUCHTE", humStr, "% rel. Feuchte", getHumColor(humidity.relative_humidity));

    // Reihe 2: CO2 & VOC
    drawSensorBox(col1_x, row2_y, COL_WIDTH, ROW_HEIGHT,
                  "CO2", co2Str, "ppm", getCO2Color(co2));
    drawSensorBox(col2_x, row2_y, COL_WIDTH, ROW_HEIGHT,
                  "VOC INDEX", vocStr, "1-500", getVOCColor(vocIndex));

    // Reihe 3: PM2.5 & PM10
    drawSensorBox(col1_x, row3_y, COL_WIDTH, ROW_HEIGHT,
                  "FEINSTAUB PM2.5", pm25Str, "ug/m3", getPM25Color(pmsData.PM_AE_UG_2_5));
    drawSensorBox(col2_x, row3_y, COL_WIDTH, ROW_HEIGHT,
                  "FEINSTAUB PM10", pm10Str, "ug/m3", getPM25Color(pmsData.PM_AE_UG_10_0));

    // Serial Ausgabe
    Serial.printf("T:%.1f H:%.0f CO2:%d VOC:%d PM2.5:%d PM10:%d\n",
                  temp.temperature, humidity.relative_humidity, co2, vocIndex,
                  pmsData.PM_AE_UG_2_5, pmsData.PM_AE_UG_10_0);
  }
}
