/**
 * @file mhz19c.h
 * @brief Driver for MH-Z19C CO2 sensor (UART)
 * @author Team InspectAir
 * @date January 2026
 * 
 * Provides functions for initialization and reading of
 * the MH-Z19C NDIR CO2 sensor via UART.
 */

#ifndef SENSORS_MHZ19C_H
#define SENSORS_MHZ19C_H

#include "../include/sensor_types.h"

// ============================================
// MH-Z19C CO2 SENSOR
// ============================================

/**
 * Initializes MH-Z19C sensor
 * @return true on successful initialization
 */
bool sensors_mhz19_init(void);

/**
 * Reads CO2 concentration
 * @param data Pointer to MHZ19C_Data structure
 * @return true on successful measurement
 */
bool sensors_mhz19_read(MHZ19C_Data* data);

#endif
