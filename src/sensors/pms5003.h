/**
 * @file pms5003.h
 * @brief Driver for PMS5003 particulate sensor (UART)
 * @author Team InspectAir
 * @date January 2026
 * 
 * Provides functions for initialization and continuous
 * reading of the PMS5003 laser particulate sensor.
 */

#ifndef SENSORS_PMS5003_H
#define SENSORS_PMS5003_H

#include "../include/sensor_types.h"

// ============================================
// PMS5003 PARTICULATE SENSOR
// ============================================

/**
 * Initializes PMS5003 sensor
 * @return true on successful initialization
 */
bool sensors_pms_init(void);

/**
 * Reads particulate data (continuous)
 * @param data Pointer to PMS5003_Data structure
 * @return true when new data is available
 */
bool sensors_pms_read(PMS5003_Data* data);

#endif
