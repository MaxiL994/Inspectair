#include "mhz19c.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <MHZ19.h>

// ============================================
// MH-Z19C CO2 SENSOR IMPLEMENTATION
// ============================================
// Uses HardwareSerial(0) = UART0 because:
// - USB CDC is enabled (ARDUINO_USB_CDC_ON_BOOT=1), so Serial uses USB
// - UART0 hardware is FREE and much more reliable than SoftwareSerial
// - UART1 = PMS5003 (Particulate)
// - UART2 = LD2410C (Radar)

static HardwareSerial SerialCO2(0); // UART0 - free because Serial uses USB CDC
static MHZ19 myMHZ19;
static int co2Orientation = 0; // 0: RX=PIN_CO2_RX/TX=PIN_CO2_TX, 1: swapped
static unsigned long lastCo2Retry = 0;

// Send raw MH-Z19C "Read CO2" command and check for ANY response.
// Returns true if at least 1 byte comes back (proving wiring works).
static bool mhz19_raw_test() {
  // Flush RX buffer
  while (SerialCO2.available()) SerialCO2.read();

  // MH-Z19C read-CO2 command (9 bytes)
  const uint8_t cmd[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  SerialCO2.write(cmd, 9);
  SerialCO2.flush();  // wait until TX complete

  // Wait up to 2 s for any response bytes
  unsigned long t0 = millis();
  while (millis() - t0 < 2000) {
    int avail = SerialCO2.available();
    if (avail >= 9) {
      uint8_t resp[9];
      SerialCO2.readBytes(resp, 9);
      Serial.printf("    [MHZ19C] Raw response (%d bytes): ", avail);
      for (int i = 0; i < 9; i++) Serial.printf("%02X ", resp[i]);
      Serial.println();
      // Validate: first byte should be 0xFF, second 0x86
      if (resp[0] == 0xFF && resp[1] == 0x86) {
        uint16_t co2 = (resp[2] << 8) | resp[3];
        Serial.printf("    [MHZ19C] Raw CO2 = %d ppm\n", co2);
      }
      return true;
    }
    delay(10);
  }
  // Check partial response
  int avail = SerialCO2.available();
  if (avail > 0) {
    Serial.printf("    [MHZ19C] Partial response (%d bytes): ", avail);
    while (SerialCO2.available()) Serial.printf("%02X ", SerialCO2.read());
    Serial.println();
    return true;
  }
  Serial.println("    [MHZ19C] Raw test: 0 bytes received (TX or RX wire missing!)");
  return false;
}

static void mhz19_begin_orientation(int orientation) {
  SerialCO2.end();
  delay(50);
  if (orientation == 0) {
    SerialCO2.begin(9600, SERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX);
  } else {
    SerialCO2.begin(9600, SERIAL_8N1, PIN_CO2_TX, PIN_CO2_RX);
  }
  delay(300);
  myMHZ19.begin(SerialCO2);
  myMHZ19.autoCalibration(false);
}

bool sensors_mhz19_init(void) {
  // Try BOTH orientations - send a command and see which one responds
  int rxPins[] = {PIN_CO2_RX, PIN_CO2_TX};
  int txPins[] = {PIN_CO2_TX, PIN_CO2_RX};
  const char* labels[] = {
    "ESP-RX=GPIO4, ESP-TX=GPIO5",
    "ESP-RX=GPIO5, ESP-TX=GPIO4 (swapped)"
  };
  
  for (int i = 0; i < 2; i++) {
    Serial.printf("    [MHZ19C] Try %d: UART0 %s, 9600 baud\n", i+1, labels[i]);
    mhz19_begin_orientation(i);
    
    int32_t testCO2 = myMHZ19.getCO2();
    Serial.printf("    [MHZ19C] CO2 reading: %d ppm (errorCode: %d)\n", testCO2, myMHZ19.errorCode);
    
    if (testCO2 > 0) {
      co2Orientation = i;
      Serial.printf("  MH-Z19C: OK with %s\n", labels[i]);
      return true;
    }
    
    // Check if we got ANY response (even error) - that means wiring is right
    if (myMHZ19.errorCode == RESULT_OK || myMHZ19.errorCode == RESULT_FILTER) {
      co2Orientation = i;
      Serial.printf("  MH-Z19C: Communication OK, warming up (~3 min). Using %s\n", labels[i]);
      return true;
    }
  }
  
  // Both orientations via library failed.
  // Run raw byte test on both orientations to distinguish code vs wiring.
  Serial.println("    [MHZ19C] Library failed. Running raw byte diagnostic...");
  for (int i = 0; i < 2; i++) {
    mhz19_begin_orientation(i);
    Serial.printf("    [MHZ19C] Raw test orientation %d (ESP-RX=GPIO%d, ESP-TX=GPIO%d):\n",
                  i, i == 0 ? PIN_CO2_RX : PIN_CO2_TX,
                  i == 0 ? PIN_CO2_TX : PIN_CO2_RX);
    if (mhz19_raw_test()) {
      co2Orientation = i;
      Serial.println("  MH-Z19C: Raw bytes received! Wiring OK, sensor may need warmup (~3 min).");
      return true;
    }
  }
  Serial.println("  MH-Z19C: WARNING - No bytes on either orientation. Check TX/RX wiring!");
  return true;
}

bool sensors_mhz19_read(MHZ19C_Data* data) {
  if (!data) return false;

  int32_t co2 = myMHZ19.getCO2();
  data->co2_ppm = co2;
  data->valid = (co2 > 0);

  // Automatic recovery if no UART response for longer period
  if (!data->valid && myMHZ19.errorCode == RESULT_TIMEOUT) {
    if (millis() - lastCo2Retry > 30000) {
      lastCo2Retry = millis();
      co2Orientation = 1 - co2Orientation;
      Serial.printf("[MHZ19C] Retry: switching UART orientation to %s\n",
                    co2Orientation == 0 ? "RX=4/TX=5" : "RX=5/TX=4");
      mhz19_begin_orientation(co2Orientation);
    }
  }

  return data->valid;
}
