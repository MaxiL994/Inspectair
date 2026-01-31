#include "mhz19c.h"
#include "../include/pins.h"
#include <SoftwareSerial.h>  // Software Serial für CO2 (UART2 wird vom Radar benutzt)
#include <MHZ19.h>

// ============================================
// MH-Z19C CO2 SENSOR IMPLEMENTATION
// ============================================
// Verwendet SoftwareSerial weil:
// - UART0 = USB Serial (Monitor)
// - UART1 = PMS5003 (Feinstaub)
// - UART2 = LD2410C (Radar)
// CO2 Sensor läuft mit 9600 Baud, das ist langsam genug für SoftwareSerial

static SoftwareSerial SerialCO2;
static MHZ19 myMHZ19;

bool sensors_mhz19_init(void) {
  Serial.printf("    [MHZ19C] Initialisiere SoftwareSerial auf RX=%d, TX=%d (9600 Baud)...\n", 
                PIN_CO2_RX, PIN_CO2_TX);
  SerialCO2.begin(9600, SWSERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX, false);
  delay(1000);
  
  myMHZ19.begin(SerialCO2);
  myMHZ19.autoCalibration(false);
  
  // Teste ob der Sensor antwortet
  int32_t testCO2 = myMHZ19.getCO2();
  Serial.printf("    [MHZ19C] Test-Lesung: %d ppm\n", testCO2);
  
  if (testCO2 > 0) {
    Serial.println("  MH-Z19C: OK");
  } else {
    Serial.println("  MH-Z19C: WARNUNG - Keine Antwort (Aufwärmzeit ~3min)");
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
