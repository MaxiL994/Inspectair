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
static int pmsOrientation = 0; // 0: RX=PIN_PMS_RX/TX=PIN_PMS_TX, 1: swapped
static unsigned long lastPmsFrame = 0;
static unsigned long lastPmsRetry = 0;

static void pms_begin_orientation(int orientation) {
  SerialPMS.end();
  delay(100);
  if (orientation == 0) {
    SerialPMS.begin(9600, SERIAL_8N1, PIN_PMS_RX, PIN_PMS_TX);
  } else {
    SerialPMS.begin(9600, SERIAL_8N1, PIN_PMS_TX, PIN_PMS_RX);
  }
  pms.wakeUp();
  pms.activeMode();
  delay(200);
}

bool sensors_pms_init(void) {
  // PIN_MAPPING says: PMS_RX(16) = "PMS5003 RX (ESP empfängt)" 
  // This is AMBIGUOUS. Radar confirmed: pin names = ESP-side.
  // But let's try BOTH orientations and use whichever works.
  
  // Try orientation 1: ESP RX=16, TX=17
  Serial.printf("    [PMS5003] Try 1: UART1 ESP-RX=GPIO%d, ESP-TX=GPIO%d\n", PIN_PMS_RX, PIN_PMS_TX);
  pms_begin_orientation(0);
  
  Serial.println("    [PMS5003] Waiting for data (up to 5s)...");
  unsigned long start = millis();
  while (millis() - start < 5000) {
    if (SerialPMS.available() > 0) {
      int avail = SerialPMS.available();
      pmsOrientation = 0;
      lastPmsFrame = millis();
      Serial.printf("  PMS5003: OK (%d bytes after %lums)\n", avail, millis() - start);
      return true;
    }
    delay(100);
  }
  Serial.println("    [PMS5003] No data on orientation 1.");
  
  // Try orientation 2: ESP RX=17, TX=16 (swapped)
  Serial.printf("    [PMS5003] Try 2: UART1 ESP-RX=GPIO%d, ESP-TX=GPIO%d (swapped)\n", PIN_PMS_TX, PIN_PMS_RX);
  pms_begin_orientation(1);
  
  start = millis();
  while (millis() - start < 5000) {
    if (SerialPMS.available() > 0) {
      int avail = SerialPMS.available();
      pmsOrientation = 1;
      lastPmsFrame = millis();
      Serial.printf("  PMS5003: OK with swapped pins (%d bytes after %lums)\n", avail, millis() - start);
      return true;
    }
    delay(100);
  }
  Serial.println("    [PMS5003] No data on orientation 2 either.");
  
  Serial.println("  PMS5003: WARNING - No UART data! Check TX wire from sensor to ESP.");
  return true;
}

bool sensors_pms_read(PMS5003_Data* data) {
  if (!data) return false;

  // Kontinuierlich lesen
  if (pms.read(pmsData)) {
    lastPmsFrame = millis();
    data->PM_AE_UG_1_0 = pmsData.PM_AE_UG_1_0;
    data->PM_AE_UG_2_5 = pmsData.PM_AE_UG_2_5;
    data->PM_AE_UG_10_0 = pmsData.PM_AE_UG_10_0;
    data->PM_SP_UG_1_0 = pmsData.PM_SP_UG_1_0;
    data->PM_SP_UG_2_5 = pmsData.PM_SP_UG_2_5;
    data->PM_SP_UG_10_0 = pmsData.PM_SP_UG_10_0;
    return true;
  }

  // Recovery: no frame for long time -> switch orientation and re-init
  if (millis() - lastPmsFrame > 30000 && millis() - lastPmsRetry > 30000) {
    lastPmsRetry = millis();
    pmsOrientation = 1 - pmsOrientation;
    Serial.printf("[PMS5003] Retry: switching UART orientation to %s\n",
                  pmsOrientation == 0 ? "RX=16/TX=17" : "RX=17/TX=16");
    pms_begin_orientation(pmsOrientation);
  }

  return false;
}
