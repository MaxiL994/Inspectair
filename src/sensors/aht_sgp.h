/**
 * @file aht_sgp.h
 * @brief Driver for AHT20 (temperature/humidity) and SGP40 (VOC) I2C sensors
 * @author Team InspectAir
 * @date January 2026
 * 
 * Provides functions for initialization and reading of
 * the I2C sensors AHT20 and SGP40.
 */

#ifndef SENSORS_AHT_SGP_H
#define SENSORS_AHT_SGP_H

#include "../include/sensor_types.h"

// ============================================
// I2C SENSORS - AHT20 & SGP40
// ============================================

/**
 * Initializes AHT20 and SGP40 sensors
 * @return true if both initialized successfully
 */
bool sensors_i2c_init(void);

/**
 * Reads AHT20 temperature and humidity
 * @param data Pointer to AHT20_Data structure
 * @return true on successful measurement
 */
bool sensors_aht20_read(AHT20_Data* data);

/**
 * Reads SGP40 VOC index
 * @param temperature Current temperature for calibration
 * @param humidity Current humidity for calibration
 * @param data Pointer to SGP40_Data structure
 * @return true on successful measurement
 */
bool sensors_sgp40_read(float temperature, float humidity, SGP40_Data* data);

#endif
