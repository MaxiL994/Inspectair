#include "aht_sgp.h"
#include "pins.h"
#include "debug_log.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SGP40.h>

static Adafruit_AHTX0 aht;
static Adafruit_SGP40 sgp;

bool sensors_i2c_init(void) {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(500);

  bool ahtOK = aht.begin();
  bool sgpOK = sgp.begin();
  
  if (ahtOK) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    LOG_I("I2C", "AHT20 OK (T=%.1f°C H=%.0f%%)", temp.temperature, humidity.relative_humidity);
  } else {
    LOG_E("I2C", "AHT20 FEHLER!");
  }
  
  LOG_I("I2C", "SGP40 %s", sgpOK ? "OK" : "FEHLER!");
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

  if (temperature < -40 || temperature > 85) {
    LOG_W("SGP40", "Ungültige Temp: %.1f°C", temperature);
    return false;
  }
  if (humidity < 0 || humidity > 100) {
    LOG_W("SGP40", "Ungültige Feuchte: %.0f%%", humidity);
    return false;
  }

  int32_t vocIndex = sgp.measureVocIndex(temperature, humidity);
  data->voc_index = vocIndex;

  return vocIndex > 0;
}
