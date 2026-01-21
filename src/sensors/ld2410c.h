#ifndef SENSORS_LD2410_H
#define SENSORS_LD2410_H

#include "../include/sensor_types.h"

// ============================================
// LD2410C RADAR SENSOR
// ============================================

/**
 * Initialisiert LD2410C Radar-Sensor
 * @return true bei erfolgreicher Initialisierung
 */
bool sensors_radar_init(void);

/**
 * Liest Radar-Daten (Präsenz und Bewegung)
 * @param data Zeiger auf LD2410C_Data Struktur
 * @return true bei erfolgreicher Messung
 */
bool sensors_radar_read(LD2410C_Data* data);

#endif
