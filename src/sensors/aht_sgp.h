#ifndef SENSORS_AHT_SGP_H
#define SENSORS_AHT_SGP_H

#include "../include/sensor_types.h"

// ============================================
// I2C SENSOREN - AHT20 & SGP40
// ============================================

/**
 * Initialisiert AHT20 und SGP40 Sensoren
 * @return true wenn beide erfolgreich initialisiert
 */
bool sensors_i2c_init(void);

/**
 * Liest AHT20 Temperatur und Luftfeuchtigkeit
 * @param data Zeiger auf AHT20_Data Struktur
 * @return true bei erfolgreicher Messung
 */
bool sensors_aht20_read(AHT20_Data* data);

/**
 * Liest SGP40 VOC-Index
 * @param temperature Aktuelle Temperatur für Kalibrierung
 * @param humidity Aktuelle Luftfeuchtigkeit für Kalibrierung
 * @param data Zeiger auf SGP40_Data Struktur
 * @return true bei erfolgreicher Messung
 */
bool sensors_sgp40_read(float temperature, float humidity, SGP40_Data* data);

#endif
