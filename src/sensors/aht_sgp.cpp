#include "aht_sgp.h"
#include "pins.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP40.h>

// ============================================
// I2C SENSORS IMPLEMENTATION
// ============================================

static Adafruit_AHTX0 aht;
static Adafruit_SGP40 sgp;

bool sensors_i2c_init(void) {
  // Initialize I2C bus
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(500);
  
  // I2C scanner - find all addresses on the bus
  Serial.println("    I2C Scanner - found addresses:");
  int count = 0;
  for (int addr = 0x01; addr < 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("      0x%02X ", addr);
      count++;
      if (count % 8 == 0) Serial.println();
    }
  }
  if (count == 0) Serial.println("      No devices found!");
  Serial.println("\n");

  // Initialize AHT20 (default address 0x38)
  bool ahtOK = aht.begin();
  Serial.printf("  AHT20 (0x38): %s\n", ahtOK ? "OK" : "ERROR");
  if (ahtOK) {
    Serial.println("    AHT20 sensor info:");
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    Serial.printf("    Test measurement: T=%.1f°C, H=%.0f%%\n", temp.temperature, humidity.relative_humidity);
  }

  // Initialize SGP40 (default address 0x59)
  bool sgpOK = sgp.begin();
  Serial.printf("  SGP40 (0x59): %s\n", sgpOK ? "OK" : "ERROR");
  if (sgpOK) {
    Serial.println("    SGP40 sensor ready");
  }

  return ahtOK && sgpOK;
}

bool sensors_aht20_read(AHT20_Data* data) {
  if (!data) return false;

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  data->temperature = temp.temperature;
  data->humidity = humidity.relative_humidity;

  return true;
}

bool sensors_sgp40_read(float temperature, float humidity, SGP40_Data* data) {
  if (!data) return false;

  // Validate input parameters
  if (temperature < -40 || temperature > 85) {
    Serial.printf("    [SGP40] Invalid temperature: %.1f°C\n", temperature);
    return false;
  }
  if (humidity < 0 || humidity > 100) {
    Serial.printf("    [SGP40] Invalid humidity: %.0f%%\n", humidity);
    return false;
  }

  int32_t vocIndex = sgp.measureVocIndex(temperature, humidity);
  data->voc_index = vocIndex;
  
  // Debug: show VOC value
  if (vocIndex <= 0) {
    Serial.printf("    [SGP40] VOC index: %ld (initializing...)\n", vocIndex);
  }

  return vocIndex > 0;
}
