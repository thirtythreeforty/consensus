#ifndef COMPLICATION_H
#define COMPLICATION_H

#include <pebble.h>

typedef struct BatteryComplication BatteryComplication;
typedef struct DateComplication DateComplication;

BatteryComplication* battery_complication_create(GRect bounds, BatteryChargeState *charge);
void battery_complication_destroy(BatteryComplication *complication);
Layer* battery_complication_get_layer(BatteryComplication *complication);
void battery_complication_state_changed(BatteryComplication *complication, BatteryChargeState *charge);
void battery_complication_animate_in(BatteryComplication *complication);

DateComplication* date_complication_create(GRect bounds);
void date_complication_destroy(DateComplication *complication);
Layer* date_complication_get_layer(DateComplication *complication);
void date_complication_time_changed(DateComplication *complication, struct tm *time);
void date_complication_animate_in(DateComplication *complication);

#endif