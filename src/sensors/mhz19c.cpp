#include "mhz19c.h"
#include "../include/pins.h"
#include "../include/debug_log.h"
#include <SoftwareSerial.h>
#include <MHZ19.h>

static SoftwareSerial SerialCO2;
static MHZ19 myMHZ19;

bool sensors_mhz19_init(void) {
  SerialCO2.begin(9600, SWSERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX, false);
  delay(1000);
  
  myMHZ19.begin(SerialCO2);
  myMHZ19.autoCalibration(false);
  
  int32_t testCO2 = myMHZ19.getCO2();
  if (testCO2 > 0) {
    LOG_I("CO2", "MH-Z19C OK (%d ppm)", testCO2);
  } else {
    LOG_W("CO2", "MH-Z19C keine Antwort (Aufwärmzeit ~3min)");
  }
  return true;
}

bool sensors_mhz19_read(MHZ19C_Data* data) {
  if (!data) return false;

  int32_t co2 = myMHZ19.getCO2();
  data->co2_ppm = co2;
  data->valid = (co2 > 0);

  return true;
}
