#ifndef COMPLICATION_H
#define COMPLICATION_H

#include <pebble.h>

typedef struct BatteryComplication BatteryComplication;
typedef struct DateComplication DateComplication;
typedef struct WeatherComplication WeatherComplication;

BatteryComplication* battery_complication_create(GRect frame, BatteryChargeState *charge);
void battery_complication_destroy(BatteryComplication *complication);
Layer* battery_complication_get_layer(BatteryComplication *complication);
void battery_complication_state_changed(BatteryComplication *complication, BatteryChargeState *charge);
Animation* battery_complication_animate_in(BatteryComplication *complication);

DateComplication* date_complication_create(GRect frame);
void date_complication_destroy(DateComplication *complication);
Layer* date_complication_get_layer(DateComplication *complication);
void date_complication_time_changed(DateComplication *complication, struct tm *time);
Animation* date_complication_animate_in(DateComplication *complication);

typedef struct {
	int temp_c;
	uint8_t humidity;
	uint8_t icon;
	bool valid;
} WeatherData;

WeatherData weather_from_appmessage(DictionaryIterator *iter);
WeatherComplication* weather_complication_create(GRect frame);
void weather_complication_destroy(WeatherComplication *complication);
Layer* weather_complication_get_layer(WeatherComplication *complication);
void weather_complication_weather_changed(WeatherComplication *complication, WeatherData data);
Animation* weather_complication_animate_in(WeatherComplication *complication);

#endif
