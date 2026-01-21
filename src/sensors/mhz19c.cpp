#include "mhz19c.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <MHZ19.h>

// ============================================
// MH-Z19C CO2 SENSOR IMPLEMENTATION
// ============================================

static HardwareSerial SerialCO2(2);
static MHZ19 myMHZ19;

bool sensors_mhz19_init(void) {
  Serial.println("    [MHZ19C] Initialisiere UART2 (9600 Baud)...");
  SerialCO2.begin(9600, SERIAL_8N1, PIN_CO2_RX, PIN_CO2_TX);
  delay(2000);  // Längeres Warmup für MHZ19
  
  Serial.println("    [MHZ19C] Starte MHZ19 Objekt...");
  myMHZ19.begin(SerialCO2);
  delay(100);
  
  // Prüfe ob Sensor antwortet
  int co2 = myMHZ19.getCO2();
  Serial.printf("    [MHZ19C] Test-Messung: %d ppm\n", co2);
  
  myMHZ19.autoCalibration(false);  // Autokalibrierung deaktivieren
  
  Serial.println("  MH-Z19C: OK");
  return true;
}

bool sensors_mhz19_read(MHZ19C_Data* data) {
  if (!data) return false;

  int32_t co2 = myMHZ19.getCO2();
  data->co2_ppm = co2;
  data->valid = (co2 > 0 && co2 < 5000);

  return data->valid;
}
