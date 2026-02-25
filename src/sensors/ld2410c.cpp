#include "ld2410c.h"
#include "../include/pins.h"
#include "../include/debug_log.h"
#include <HardwareSerial.h>
#include <ld2410.h>

static HardwareSerial SerialRadar(2);
static ld2410 radar;
static bool useOutPin = false;

static const uint32_t BAUD_RATES[] = {256000, 115200, 9600};
static const int NUM_BAUDS = 3;

bool sensors_radar_init(void) {
  pinMode(PIN_RADAR_OUT, INPUT);
  
  // Teste verschiedene Baudraten
  for (int b = 0; b < NUM_BAUDS; b++) {
    uint32_t baud = BAUD_RATES[b];
    
    SerialRadar.end();
    delay(100);
    SerialRadar.begin(baud, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
    delay(500);
    
    while (SerialRadar.available()) SerialRadar.read();
    delay(200);
    
    // Warte auf Daten
    int bytesAvailable = 0;
    for (int i = 0; i < 10; i++) {
      delay(100);
      bytesAvailable = SerialRadar.available();
      if (bytesAvailable > 0) {
        // Buffer leeren und Radar initialisieren
        while (SerialRadar.available()) SerialRadar.read();
        SerialRadar.end();
        delay(100);
        SerialRadar.begin(baud, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
        delay(200);
        
        radar.begin(SerialRadar);
        useOutPin = false;
        LOG_I("RADAR", "LD2410C OK (UART2, %d Baud)", baud);
        return true;
      }
    }
  }
  
  // Fallback: OUT Pin
  useOutPin = true;
  LOG_W("RADAR", "UART fehlgeschlagen → OUT Pin Modus (GPIO %d)", PIN_RADAR_OUT);
  return true;
}

bool sensors_radar_read(LD2410C_Data* data) {
  if (!data) return false;

  // OUT Pin Modus (Fallback)
  if (useOutPin) {
    int outState = digitalRead(PIN_RADAR_OUT);
    data->presence = outState ? 1 : 0;
    data->motion = outState ? 1 : 0;
    data->distance = outState ? 50 : 0;
    return true;
  }

  // UART Modus
  radar.read();
  
  if (radar.isConnected()) {
    uint16_t moving_dist = radar.movingTargetDistance();
    uint16_t stationary_dist = radar.stationaryTargetDistance();
    
    uint16_t min_dist = moving_dist;
    if (stationary_dist > 0 && (min_dist == 0 || stationary_dist < min_dist)) {
      min_dist = stationary_dist;
    }
    
    bool closeDetected = (min_dist > 0 && min_dist <= 15);
    
    data->presence = closeDetected ? 1 : 0;
    data->motion = radar.movingTargetDetected() ? 1 : 0;
    data->distance = min_dist;
    
    return true;
  }

  return false;
}
