#include "mhz19c.h"
#include "../include/pins.h"
#include <SoftwareSerial.h>  // Software Serial for CO2 (UART2 used by radar)
#include <MHZ19.h>

// ============================================
// MH-Z19C CO2 SENSOR IMPLEMENTATION
// ============================================
// Uses SoftwareSerial because:
// - UART0 = USB Serial (Monitor)
// - UART1 = PMS5003 (Particulate)
// - UART2 = LD2410C (Radar)
// CO2 sensor runs at 9600 baud, slow enough for SoftwareSerial

static SoftwareSerial SerialCO2;
static MHZ19 myMHZ19;

bool sensors_mhz19_init(void) {
  Serial.printf("    [MHZ19C] Initializing SoftwareSerial on RX=%d, TX=%d (9600 baud)...\n", 
                PIN_CO2_RX, PIN_CO2_TX);
  SerialCO2.begin(9600, SWSERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX, false);
  delay(1000);
  
  myMHZ19.begin(SerialCO2);
  myMHZ19.autoCalibration(false);
  
  // Test if sensor responds
  int32_t testCO2 = myMHZ19.getCO2();
  Serial.printf("    [MHZ19C] Test reading: %d ppm\n", testCO2);
  
  if (testCO2 > 0) {
    Serial.println("  MH-Z19C: OK");
  } else {
    Serial.println("  MH-Z19C: WARNING - No response (warmup time ~3min)");
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
