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
    // Lese Präsenz-Status
    data->presence = (radar.presenceDetected() ? 1 : 0);
    
    // Lese Bewegungsdaten
    data->motion = radar.movingTargetDetected() ? 1 : 0;
    data->distance = radar.movingTargetDistance();
    
    return true;
  }

  return false;
}
