#include "ld2410c.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <ld2410.h>

// ============================================
// LD2410C RADAR SENSOR IMPLEMENTATION
// ============================================

static HardwareSerial SerialRadar(1); // ÄNDERUNG: Nutze UART1 statt UART0
static ld2410 radar;

bool sensors_radar_init(void) {
  // Einfache Initialisierung mit Standard-Baudrate 256000
  Serial.println("    [LD2410C] Initialisiere UART1 (256000 Baud)...");
  SerialRadar.begin(256000, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  delay(1000);
  
  radar.begin(SerialRadar);
  Serial.println("  LD2410C: OK");
  return true;
}

bool sensors_radar_read(LD2410C_Data* data) {
  if (!data) return false;

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
      Serial.println("[RADAR] Sensor nicht verbunden!");
    }
  }

  return false;
}
