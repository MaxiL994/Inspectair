/**
 * @file ld2410c.h
 * @brief Driver for LD2410C radar presence sensor (UART)
 * @author Team InspectAir
 * @date January 2026
 * 
 * Provides functions for initialization and reading of
 * the LD2410C mmWave radar sensor for presence and motion detection.
 */

#ifndef SENSORS_LD2410_H
#define SENSORS_LD2410_H

#include "../include/sensor_types.h"

// ============================================
// LD2410C RADAR SENSOR
// ============================================

/**
 * Initializes LD2410C radar sensor
 * @return true on successful initialization
 */
bool sensors_radar_init(void);

/**
 * Reads radar data (presence and motion)
 * @param data Pointer to LD2410C_Data structure
 * @return true on successful measurement
 */
bool sensors_radar_read(LD2410C_Data* data);

#endif
