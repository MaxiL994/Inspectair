#ifndef SENSOR_TYPES_H
#define SENSOR_TYPES_H

#include <stdint.h>

// ============================================
// SENSOR DATENSTRUKTUREN
// ============================================

typedef struct {
  float temperature;
  float humidity;
} AHT20_Data;

typedef struct {
  int32_t voc_index;
  uint16_t raw_value;
} SGP40_Data;

typedef struct {
  uint16_t PM_AE_UG_1_0;
  uint16_t PM_AE_UG_2_5;
  uint16_t PM_AE_UG_10_0;
  uint16_t PM_SP_UG_1_0;
  uint16_t PM_SP_UG_2_5;
  uint16_t PM_SP_UG_10_0;
} PMS5003_Data;

typedef struct {
  int32_t co2_ppm;
  bool valid;
} MHZ19C_Data;

typedef struct {
  uint8_t presence;
  uint16_t distance;
  uint8_t motion;
} LD2410C_Data;

typedef struct {
  AHT20_Data aht;
  SGP40_Data sgp;
  PMS5003_Data pms;
  MHZ19C_Data mhz;
  LD2410C_Data radar;
  uint32_t timestamp;
} SensorReadings;

#endif
