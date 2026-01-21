#include "ld2410c.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <ld2410.h>

// ============================================
// LD2410C RADAR SENSOR IMPLEMENTATION
// ============================================

static HardwareSerial SerialRadar(0);
static ld2410 radar;

bool sensors_radar_init(void) {
  // Radar nutzt UART0 (via Level Shifter)
  Serial.println("    [LD2410C] Initialisiere UART0 (115200 Baud)...");
  SerialRadar.begin(115200, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  delay(2000);  // Längeres Warmup für Radar
  
  Serial.println("    [LD2410C] Starte LD2410 Objekt...");
  radar.begin(SerialRadar);
  delay(500);
  
  // Teste ob Sensor antwortet
  if (radar.read()) {
    Serial.println("    [LD2410C] Sensor antwortet!");
  } else {
    Serial.println("    [LD2410C] WARNUNG: Sensor antwortet nicht!");
  }
  
  Serial.println("  LD2410C: OK");
  return true;
}

bool sensors_radar_read(LD2410C_Data* data) {
  if (!data) return false;

  if (radar.read()) {
    // Lese Präsenz-Status
    data->presence = (radar.presenceDetected() ? 1 : 0);
    
    // Lese Bewegungsdaten
    data->motion = radar.movingTargetDetected() ? 1 : 0;
    data->distance = radar.movingTargetDistance();
    
    return true;
  }

  return false;
}
