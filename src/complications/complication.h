#ifndef COMPLICATION_H
#define COMPLICATION_H

#include <pebble.h>

typedef struct BatteryComplication BatteryComplication;

BatteryComplication* battery_complication_create(GRect bounds, BatteryChargeState *charge);
void battery_complication_destroy(BatteryComplication *complication);
Layer* battery_complication_get_layer(BatteryComplication *complication);
void battery_complication_state_changed(BatteryComplication *complication, BatteryChargeState *charge);
void battery_complication_animate_in(BatteryComplication *complication);

#endif
