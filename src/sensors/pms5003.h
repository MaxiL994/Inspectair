/**
 * @file pms5003.h
 * @brief Treiber für PMS5003 Feinstaub-Sensor (UART)
 * @author Team InspectAir
 * @date Januar 2026
 * 
 * Stellt Funktionen zur Initialisierung und zum kontinuierlichen
 * Auslesen des PMS5003 Laser-Feinstaubsensors bereit.
 */

#ifndef SENSORS_PMS5003_H
#define SENSORS_PMS5003_H

#include "../include/sensor_types.h"

// ============================================
// PMS5003 FEINSTAUB SENSOR
// ============================================

/**
 * Initialisiert PMS5003 Sensor
 * @return true bei erfolgreicher Initialisierung
 */
bool sensors_pms_init(void);

/**
 * Liest Feinstaubdaten (kontinuierlich)
 * @param data Zeiger auf PMS5003_Data Struktur
 * @return true wenn neue Daten verfügbar
 */
bool sensors_pms_read(PMS5003_Data* data);

#endif
