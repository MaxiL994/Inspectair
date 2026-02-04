#include "pms5003.h"
#include "../include/pins.h"
#include <HardwareSerial.h>
#include <PMS.h>

// ============================================
// PMS5003 PARTICULATE SENSOR IMPLEMENTATION
// ============================================

static HardwareSerial SerialPMS(1); // UART1 for PMS5003
static PMS pms(SerialPMS);
static PMS::DATA pmsData;

bool sensors_pms_init(void) {
  SerialPMS.begin(9600, SERIAL_8N1, PIN_PMS_RX, PIN_PMS_TX);
  delay(500);
  
  Serial.println("  PMS5003: OK");
  return true;
}

bool sensors_pms_read(PMS5003_Data* data) {
  if (!data) return false;

  // Kontinuierlich lesen
  if (pms.read(pmsData)) {
    data->PM_AE_UG_1_0 = pmsData.PM_AE_UG_1_0;
    data->PM_AE_UG_2_5 = pmsData.PM_AE_UG_2_5;
    data->PM_AE_UG_10_0 = pmsData.PM_AE_UG_10_0;
    data->PM_SP_UG_1_0 = pmsData.PM_SP_UG_1_0;
    data->PM_SP_UG_2_5 = pmsData.PM_SP_UG_2_5;
    data->PM_SP_UG_10_0 = pmsData.PM_SP_UG_10_0;
    return true;
  }

  return false;
}
