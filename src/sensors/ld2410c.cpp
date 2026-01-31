#include "ld2410c.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <ld2410.h>

// ============================================
// LD2410C RADAR SENSOR IMPLEMENTATION
// ============================================

static HardwareSerial SerialRadar(2); // UART2 für Radar
static ld2410 radar;
static bool useOutPin = false;  // Fallback auf OUT Pin wenn UART nicht geht

// Teste verschiedene Baudraten
static const uint32_t BAUD_RATES[] = {256000, 115200, 9600};
static const int NUM_BAUDS = 3;

bool sensors_radar_init(void) {
  Serial.printf("    [LD2410C] Pins: RX=%d, TX=%d, OUT=%d\n", PIN_RADAR_RX, PIN_RADAR_TX, PIN_RADAR_OUT);
  
  // Konfiguriere OUT Pin als Fallback
  pinMode(PIN_RADAR_OUT, INPUT);
  
  // Teste verschiedene Baudraten
  for (int b = 0; b < NUM_BAUDS; b++) {
    uint32_t baud = BAUD_RATES[b];
    Serial.printf("    [LD2410C] Teste %d Baud...", baud);
    
    SerialRadar.end();
    delay(100);
    SerialRadar.begin(baud, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
    delay(500);
    
    // Leere Buffer
    while (SerialRadar.available()) SerialRadar.read();
    delay(200);
    
    // Warte auf Daten
    int bytesAvailable = 0;
    for (int i = 0; i < 10; i++) {
      delay(100);
      bytesAvailable = SerialRadar.available();
      if (bytesAvailable > 0) {
        Serial.printf(" %d Bytes empfangen!\n", bytesAvailable);
        
        // Zeige erste Bytes (Hex)
        Serial.print("    [LD2410C] Daten: ");
        for (int j = 0; j < min(bytesAvailable, 16); j++) {
          Serial.printf("%02X ", SerialRadar.read());
        }
        Serial.println();
        
        // Initialisiere mit dieser Baudrate
        SerialRadar.end();
        delay(100);
        SerialRadar.begin(baud, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
        delay(200);
        
        radar.begin(SerialRadar);
        useOutPin = false;
        Serial.printf("  LD2410C: OK mit UART %d Baud\n", baud);
        return true;
      }
    }
    Serial.println(" keine Daten");
  }
  
  // UART funktioniert nicht - teste OUT Pin
  Serial.println("    [LD2410C] UART fehlgeschlagen, teste OUT Pin...");
  
  // Prüfe ob OUT Pin reagiert (sollte LOW oder HIGH sein)
  int outState = digitalRead(PIN_RADAR_OUT);
  Serial.printf("    [LD2410C] OUT Pin (GPIO %d) Status: %s\n", 
                PIN_RADAR_OUT, outState ? "HIGH (Bewegung)" : "LOW (keine Bewegung)");
  
  useOutPin = true;
  Serial.println("  LD2410C: Nutze OUT Pin Modus (nur Bewegungserkennung, keine Distanz)");
  return true;
}

bool sensors_radar_read(LD2410C_Data* data) {
  if (!data) return false;

  // OUT Pin Modus (Fallback wenn UART nicht funktioniert)
  if (useOutPin) {
    static unsigned long lastOutDebug = 0;
    int outState = digitalRead(PIN_RADAR_OUT);
    
    data->presence = outState ? 1 : 0;
    data->motion = outState ? 1 : 0;
    data->distance = outState ? 50 : 0;  // Dummy-Distanz
    
    // Debug alle 5 Sekunden
    if (millis() - lastOutDebug > 5000) {
      lastOutDebug = millis();
      Serial.printf("[RADAR-OUT] GPIO %d = %s\n", PIN_RADAR_OUT, outState ? "HIGH (Bewegung!)" : "LOW");
    }
    
    return true;
  }

  // UART Modus
  // Debug: Zeige alle 10 Sekunden ob rohe Bytes ankommen
  static unsigned long lastRawCheck = 0;
  if (millis() - lastRawCheck > 10000) {
    lastRawCheck = millis();
    int avail = SerialRadar.available();
    Serial.printf("[RADAR-DEBUG] Raw bytes available: %d\n", avail);
    if (avail > 0) {
      Serial.print("[RADAR-DEBUG] First bytes: ");
      for (int i = 0; i < min(avail, 20); i++) {
        Serial.printf("%02X ", SerialRadar.peek());
        SerialRadar.read();
      }
      Serial.println();
    }
  }

  radar.read(); // Standard read
  
  if (radar.isConnected()) {
    // Lese Distanz (in cm)
    uint16_t moving_dist = radar.movingTargetDistance();
    uint16_t stationary_dist = radar.stationaryTargetDistance();
    
    // Debug alle 5 Sekunden
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 5000) {
      lastDebug = millis();
      Serial.printf("[RADAR] Moving: %dcm, Stationary: %dcm, Connected: %d\n", 
                    moving_dist, stationary_dist, radar.isConnected());
    }
    
    // Minimum der beiden Distanzen nehmen
    uint16_t min_dist = moving_dist;
    if (stationary_dist > 0 && (min_dist == 0 || stationary_dist < min_dist)) {
      min_dist = stationary_dist;
    }
    
    // Auslösen bei 10cm oder näher
    data->presence = (min_dist > 0 && min_dist <= 10) ? 1 : 0;
    data->motion = radar.movingTargetDetected() ? 1 : 0;
    data->distance = min_dist;
    
    return true;
  } else {
    static unsigned long lastWarn = 0;
    if (millis() - lastWarn > 10000) {
      lastWarn = millis();
      Serial.println("[RADAR] UART nicht verbunden, nutze OUT Pin");
    }
  }

  return false;
}
