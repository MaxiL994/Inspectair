#include "ld2410c.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <ld2410.h>

// ============================================
// LD2410C RADAR SENSOR IMPLEMENTATION
// ============================================

static HardwareSerial SerialRadar(2); // UART2 for radar
static ld2410 radar;
static bool useOutPin = false;  // Fallback to OUT pin if UART fails

// Test different baud rates
static const uint32_t BAUD_RATES[] = {256000, 115200, 9600};
static const int NUM_BAUDS = 3;

bool sensors_radar_init(void) {
  Serial.printf("    [LD2410C] Pins: RX=%d, TX=%d, OUT=%d\n", PIN_RADAR_RX, PIN_RADAR_TX, PIN_RADAR_OUT);
  
  // Configure OUT pin as fallback
  pinMode(PIN_RADAR_OUT, INPUT);
  
  // Test different baud rates
  for (int b = 0; b < NUM_BAUDS; b++) {
    uint32_t baud = BAUD_RATES[b];
    Serial.printf("    [LD2410C] Testing %d baud...", baud);
    
    SerialRadar.end();
    delay(100);
    SerialRadar.begin(baud, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
    delay(500);
    
    // Clear buffer
    while (SerialRadar.available()) SerialRadar.read();
    delay(200);
    
    // Wait for data
    int bytesAvailable = 0;
    for (int i = 0; i < 10; i++) {
      delay(100);
      bytesAvailable = SerialRadar.available();
      if (bytesAvailable > 0) {
        Serial.printf(" %d bytes received!\n", bytesAvailable);
        
        // Show first bytes (hex)
        Serial.print("    [LD2410C] Data: ");
        for (int j = 0; j < min(bytesAvailable, 16); j++) {
          Serial.printf("%02X ", SerialRadar.read());
        }
        Serial.println();
        
        // Initialize with this baud rate
        SerialRadar.end();
        delay(100);
        SerialRadar.begin(baud, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
        delay(200);
        
        radar.begin(SerialRadar);
        useOutPin = false;
        Serial.printf("  LD2410C: OK with UART %d baud\n", baud);
        return true;
      }
    }
    Serial.println(" no data");
  }
  
  // UART failed - test OUT pin
  Serial.println("    [LD2410C] UART failed, testing OUT pin...");
  
  // Check if OUT pin responds (should be LOW or HIGH)
  int outState = digitalRead(PIN_RADAR_OUT);
  Serial.printf("    [LD2410C] OUT pin (GPIO %d) status: %s\n", 
                PIN_RADAR_OUT, outState ? "HIGH (motion)" : "LOW (no motion)");
  
  useOutPin = true;
  Serial.println("  LD2410C: Using OUT pin mode (motion detection only, no distance)");
  return true;
}

bool sensors_radar_read(LD2410C_Data* data) {
  if (!data) return false;

  // OUT pin mode (fallback when UART fails)
  if (useOutPin) {
    static unsigned long lastOutDebug = 0;
    int outState = digitalRead(PIN_RADAR_OUT);
    
    data->presence = outState ? 1 : 0;
    data->motion = outState ? 1 : 0;
    data->distance = outState ? 50 : 0;  // Dummy distance
    
    // Debug every 5 seconds
    if (millis() - lastOutDebug > 5000) {
      lastOutDebug = millis();
      Serial.printf("[RADAR-OUT] GPIO %d = %s\n", PIN_RADAR_OUT, outState ? "HIGH (motion!)" : "LOW");
    }
    
    return true;
  }

  // UART mode
  // Debug: show every 10 seconds if raw bytes arrive
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
    // Read distance (in cm)
    uint16_t moving_dist = radar.movingTargetDistance();
    uint16_t stationary_dist = radar.stationaryTargetDistance();
    
    // Take minimum of both distances
    uint16_t min_dist = moving_dist;
    if (stationary_dist > 0 && (min_dist == 0 || stationary_dist < min_dist)) {
      min_dist = stationary_dist;
    }
    
    // Detection: closer than 10cm
    bool closeDetected = (min_dist > 0 && min_dist <= 10);
    
    // Debug: output immediately when something closer than 10cm
    static bool lastCloseState = false;
    if (closeDetected && !lastCloseState) {
      Serial.printf("[RADAR] >>> OBJECT DETECTED! Distance: %d cm <<<\n", min_dist);
    } else if (!closeDetected && lastCloseState) {
      Serial.printf("[RADAR] Object moved away (distance: %d cm)\n", min_dist);
    }
    lastCloseState = closeDetected;
    
    // Additionally: output continuously when close (every 500ms)
    static unsigned long lastCloseDebug = 0;
    if (closeDetected && (millis() - lastCloseDebug > 500)) {
      lastCloseDebug = millis();
      Serial.printf("[RADAR] CLOSE: %d cm (Moving: %d, Stationary: %d)\n", 
                    min_dist, moving_dist, stationary_dist);
    }
    
    // Standard debug every 5 seconds
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 5000) {
      lastDebug = millis();
      Serial.printf("[RADAR] Status: %dcm | Moving: %dcm, Stationary: %dcm\n", 
                    min_dist, moving_dist, stationary_dist);
    }
    
    data->presence = closeDetected ? 1 : 0;
    data->motion = radar.movingTargetDetected() ? 1 : 0;
    data->distance = min_dist;
    
    return true;
  } else {
    static unsigned long lastWarn = 0;
    if (millis() - lastWarn > 10000) {
      lastWarn = millis();
      Serial.println("[RADAR] UART not connected, using OUT pin");
    }
  }

  return false;
}
