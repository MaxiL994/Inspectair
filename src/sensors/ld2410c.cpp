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
static unsigned long lastRadarFrame = 0;
static LD2410C_Data lastGoodRadar = {0};
static bool haveLastGoodRadar = false;
static bool radarAutoTaskStarted = false;

// Test different baud rates
static const uint32_t BAUD_RATES[] = {256000, 115200, 9600};
static const int NUM_BAUDS = 3;

bool sensors_radar_init(void) {
  Serial.printf("    [LD2410C] Pins: RX=%d, TX=%d, OUT=%d\n", PIN_RADAR_RX, PIN_RADAR_TX, PIN_RADAR_OUT);
  
  // Configure OUT pin as fallback
  pinMode(PIN_RADAR_OUT, INPUT);
  
  // Pin names in pins.h are ESP-side (confirmed: original orientation works)
  // Try original first, then swapped as fallback
  int rxPins[] = {PIN_RADAR_RX, PIN_RADAR_TX};
  int txPins[] = {PIN_RADAR_TX, PIN_RADAR_RX};
  const char* pinLabels[] = {"original", "swapped"};

  for (int p = 0; p < 2; p++) {
    if (p == 1) Serial.println("    [LD2410C] Trying swapped RX/TX pins...");

    // Test different baud rates
    for (int b = 0; b < NUM_BAUDS; b++) {
      uint32_t baud = BAUD_RATES[b];
      Serial.printf("    [LD2410C] Testing %d baud (%s)...", baud, pinLabels[p]);

      SerialRadar.end();
      delay(100);
      SerialRadar.begin(baud, SERIAL_8N1, rxPins[p], txPins[p]);
      delay(500);

      int bytesAvailable = 0;
      for (int i = 0; i < 20; i++) {
        delay(100);
        bytesAvailable = SerialRadar.available();
        if (bytesAvailable > 0) break;
      }

      if (bytesAvailable > 0) {
        Serial.printf(" %d bytes - starting stream mode...\n", bytesAvailable);

        // IMPORTANT:
        // begin(..., false) disables firmware request/ack handshake.
        // This keeps UART streaming functional even if ESP->sensor TX path
        // is not available, as long as sensor->ESP RX data is present.
        if (radar.begin(SerialRadar, false)) {
          useOutPin = false;
          lastRadarFrame = millis();
          // Process radar UART in background task to avoid drops when loop is busy.
          if (!radarAutoTaskStarted) {
            radar.autoReadTask(4096, 1, 1);
            radarAutoTaskStarted = true;
          }
          Serial.printf("  LD2410C: UART stream active (%d baud, %s)\n", baud, pinLabels[p]);
          return true;
        }

        // Should rarely happen with waitForRadar=false, but keep fallback path.
        Serial.printf("    [LD2410C] Stream init failed at %d baud\n", baud);
      } else {
        Serial.println(" no data");
      }
    }
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

  // UART mode - let library process all bytes
  if (!radarAutoTaskStarted) {
    // Manual mode: call read() ourselves
    if (radar.read()) {
      lastRadarFrame = millis();
    }
  }

  // Check connection status (library internal timeout ~1s)
  bool connected = radar.isConnected();

  // When autoReadTask is running, the background task processes frames.
  // Update our timestamp whenever the library reports connected.
  if (connected) {
    lastRadarFrame = millis();
  }

  // Hysteresis: bridge brief gaps where isConnected() flickers false.
  // With the timestamp now properly updated, this 20s window only
  // activates during genuine interruptions.
  if (!connected && (millis() - lastRadarFrame < 20000)) {
    connected = true;
  }
  
  // Debug: connection status every 10 seconds
  static unsigned long lastRawCheck = 0;
  if (millis() - lastRawCheck > 10000) {
    lastRawCheck = millis();
    Serial.printf("[RADAR] Connected: %s, Bytes available: %d\n", 
                  connected ? "yes" : "no", SerialRadar.available());
  }
  
  if (connected) {
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
    lastGoodRadar = *data;
    haveLastGoodRadar = true;
    
    return true;
  } else {
    static unsigned long lastWarn = 0;
    if (millis() - lastWarn > 10000) {
      lastWarn = millis();
      Serial.println("[RADAR] UART frame timeout, keeping last valid UART radar value");
    }

    if (haveLastGoodRadar) {
      *data = lastGoodRadar;
      return true;
    }

    // Keep UART mode active; do not auto-switch to OUT mode after successful UART init.
    data->presence = 0;
    data->motion = 0;
    data->distance = 0;
    return true;
  }
}
