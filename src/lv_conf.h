#ifndef LV_CONF_H
#define LV_CONF_H

/* Einbinden der Arduino-Funktionen für millis() */
#include <Arduino.h>

/* Aktivieren des Custom Ticks */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#endif /*LV_CONF_H*/
