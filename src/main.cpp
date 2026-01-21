#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP40.h>
#include <PMS.h>
#include <MHZ19.h>
#include <ld2410.h>

// PIN DEFINITIONEN
#define PIN_I2C_SDA 8
#define PIN_I2C_SCL 18
#define PIN_PMS_RX  16
#define PIN_PMS_TX  17
#define PIN_CO2_RX  4
#define PIN_CO2_TX  5
#define PIN_RADAR_RX 6
#define PIN_RADAR_TX 7
#define PIN_TFT_BL   3

// UART Objekte
HardwareSerial SerialPMS(0);
HardwareSerial SerialCO2(2);
HardwareSerial SerialRadar(1); 

// Sensor Objekte
Adafruit_AHTX0 aht;
Adafruit_SGP40 sgp;
PMS pms(SerialPMS);
PMS::DATA pmsData;
MHZ19 myMHZ19;
ld2410 radar;

void setup() {
  // Initialisierung
  delay(2000); // Power-up safety
  Serial.begin(115200);

  // Warten auf USB (optional, aber gut für Diagnose)
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000));

  Serial.println("\n\n--- INSPECTAIR SENSOR START ---");
  
  // 1. I2C Sensoren (AHT20 & SGP40)
  Serial.println("[INIT] I2C Sensoren...");
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(500); // Kurze Pause für Spannungsstabilisierung
  
  if (aht.begin()) {
    Serial.println("  -> AHT20 gefunden.");
  } else {
    Serial.println("  -> FEHLER: AHT20 nicht gefunden!");
  }

  if (sgp.begin()) {
    Serial.println("  -> SGP40 gefunden.");
  } else {
    Serial.println("  -> FEHLER: SGP40 nicht gefunden!");
  }
  
  // 2. UART Sensoren
  Serial.println("[INIT] UART Sensoren...");
  
  // PMS5003
  SerialPMS.begin(9600, SERIAL_8N1, PIN_PMS_RX, PIN_PMS_TX);
  Serial.println("  -> PMS5003 initialisiert.");
  delay(500); // Pause vor dem nächsten großen Verbraucher

  // MH-Z19C
  SerialCO2.begin(9600, SERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX);
  delay(1000); 
  myMHZ19.begin(SerialCO2);
  myMHZ19.autoCalibration(false); // Autokalibrierung aus (besser manuell steuern)
  
  // Diagnose: Versuche Version zu lesen
  char myVersion[4];
  myMHZ19.getVersion(myVersion);
  Serial.print("  -> MH-Z19C initialisiert. Version: ");
  Serial.print(myVersion[0]); Serial.print(myVersion[1]); Serial.println(" (Wenn leer -> RX/TX prüfen!)");

  // LD2410 Radar
  SerialRadar.setRxBufferSize(16384); // Puffer noch weiter erhöht (16KB)
  SerialRadar.begin(256000, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  delay(800); // Etwas länger warten
  radar.begin(SerialRadar);
  Serial.println("  -> LD2410 Radar initialisiert.");

  // 3. PINS
  pinMode(PIN_TFT_BL, OUTPUT);
  Serial.println("[INIT] Fertig. Starte Messung...");
}

void loop() {
  static unsigned long lastCheck = 0;
  radar.read(); // Radar muss im Loop ständig gelesen werden für Updates
  pms.read(pmsData); // PMS kontinuierlich im Hintergrund lesen (nicht blockierend)
  
  // Backlight Heartbeat (Pulsieren)
  int pwmVal = (millis() / 5) % 255; 
  if ((millis() / 5) % 510 > 255) pwmVal = 255 - pwmVal;
  analogWrite(PIN_TFT_BL, pwmVal);

  // Alle 3 Sekunden Werte ausgeben
  if (millis() - lastCheck > 3000) {
    lastCheck = millis();
    Serial.println("\n--- MESSWERTE ---");
    
    // 1. AHT20 (Temp/Hum)
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    Serial.printf("KLIMA:   %.1f °C  |  %.1f %% rH\n", temp.temperature, humidity.relative_humidity);

    // 2. SGP40 (VOC)
    int32_t vocIndex = sgp.measureVocIndex(temp.temperature, humidity.relative_humidity);
    Serial.printf("LUFT:    VOC Index: %d\n", vocIndex);

    // 3. PMS5003 (Feinstaub)
    Serial.printf("STAUB:   PM1.0: %d | PM2.5: %d | PM10: %d (ug/m3)\n", 
                  pmsData.PM_AE_UG_1_0, pmsData.PM_AE_UG_2_5, pmsData.PM_AE_UG_10_0);

    // 4. MH-Z19C (CO2)
    int co2 = myMHZ19.getCO2();
    int tempCO2 = myMHZ19.getTemperature();
    
    if (myMHZ19.errorCode != RESULT_OK) {
        Serial.println("CO2:     FEHLER - Keine Verbindung! (Kabel an Pin 4/5 tauschen)");
    } else {
        Serial.printf("CO2:     %d ppm (Temp: %d C)\n", co2, tempCO2);
    }

    // WICHTIG: Radar nochmal lesen, da der CO2-Abruf oben den Loop blockiert hat
    // und sich in der Zwischenzeit Daten im Puffer angesammelt haben.
    radar.read();

    // 5. LD2410 (Radar)
    if(radar.isConnected()) {
      Serial.printf("RADAR:   Präsenz: %s | Bewegt: %d cm | Statisch: %d cm\n", 
                    radar.presenceDetected() ? "JA" : "NEIN", radar.movingTargetDistance(), radar.stationaryTargetDistance());
    } else {
      Serial.print("RADAR:   Nicht verbunden");
      if (SerialRadar.available() > 0) {
        Serial.printf(" [DEBUG: %d Bytes empfangen - Parsing Fehler]\n", SerialRadar.available());
      } else {
        Serial.println(" [DEBUG: 0 Bytes empfangen - Kabel/Strom prüfen]");
      }
    }
  }
}
