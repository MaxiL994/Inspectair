/**
 * @file mhz19c.h
 * @brief Treiber für MH-Z19C CO2-Sensor (UART)
 * @author Team InspectAir
 * @date Januar 2026
 * 
 * Stellt Funktionen zur Initialisierung und zum Auslesen des
 * MH-Z19C NDIR CO2-Sensors über UART bereit.
 */

#ifndef SENSORS_MHZ19C_H
#define SENSORS_MHZ19C_H

#include "../include/sensor_types.h"

// ============================================
// MH-Z19C CO2 SENSOR
// ============================================

/**
 * Initialisiert MH-Z19C Sensor
 * @return true bei erfolgreicher Initialisierung
 */
bool sensors_mhz19_init(void);

/**
 * Liest CO2-Konzentration
 * @param data Zeiger auf MHZ19C_Data Struktur
 * @return true bei erfolgreicher Messung
 */
bool sensors_mhz19_read(MHZ19C_Data* data);

#endif
