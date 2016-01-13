#ifndef COMPLICATION_H
#define COMPLICATION_H

#include <pebble.h>

typedef struct BatteryComplication BatteryComplication;
typedef struct DateComplication DateComplication;
typedef struct WeatherComplication WeatherComplication;

BatteryComplication* battery_complication_create(GRect frame);
void battery_complication_destroy(BatteryComplication *complication);
Layer* battery_complication_get_layer(BatteryComplication *complication);
void battery_complication_state_changed(BatteryComplication *complication, const BatteryChargeState *charge);

DateComplication* date_complication_create(GRect frame);
void date_complication_destroy(DateComplication *complication);
Layer* date_complication_get_layer(DateComplication *complication);
void date_complication_time_changed(DateComplication *complication, struct tm *time);
Animation* date_complication_animate_in(DateComplication *complication);

typedef struct {
	bool valid;

	time_t time_updated;
	int8_t temp_c;
	uint8_t humidity;
	uint8_t icon;
} WeatherData;

void weather_from_appmessage(DictionaryIterator *iter, WeatherData *wdata);
void weather_from_persist(WeatherData *wdata);
void weather_to_persist(const WeatherData *wdata);
WeatherComplication* weather_complication_create(GRect frame);
void weather_complication_destroy(WeatherComplication *complication);
Layer* weather_complication_get_layer(WeatherComplication *complication);
void weather_complication_weather_changed(WeatherComplication *complication, const WeatherData *wdata);

typedef enum {
	COMPLICATION_TYPE_NONE,
	COMPLICATION_TYPE_BATTERY,
	COMPLICATION_TYPE_DATE,
	COMPLICATION_TYPE_WEATHER,
} ComplicationType;

typedef struct {
	void *complication;
	ComplicationType type;
} AbstractComplication;

inline void* abstract_complication_to_helper(const AbstractComplication *abs_complication, ComplicationType type)
{
	if(abs_complication->type == type) {
		return abs_complication->complication;
	}
	else {
		return NULL;
	}
}

inline void abstract_complication_from_helper(AbstractComplication *abs_complication, void *complication, ComplicationType type)
{
	*abs_complication = (AbstractComplication) {
		.complication = complication,
		.type = type
	};
}

inline void abstract_complication_destroy(AbstractComplication *abs_complication)
{
	switch(abs_complication->type) {
	case COMPLICATION_TYPE_NONE:
		break;
	case COMPLICATION_TYPE_BATTERY:
		battery_complication_destroy((BatteryComplication*)abs_complication->complication);
		break;
	case COMPLICATION_TYPE_DATE:
		date_complication_destroy((DateComplication*)abs_complication->complication);
		break;
	case COMPLICATION_TYPE_WEATHER:
		weather_complication_destroy((WeatherComplication*)abs_complication->complication);
		break;
	}
	abs_complication->type = COMPLICATION_TYPE_NONE;
}

#define abstract_complication_to_battery(abs_complication) \
	((BatteryComplication*)abstract_complication_to_helper(abs_complication, COMPLICATION_TYPE_BATTERY))
#define abstract_complication_to_date(abs_complication) \
	((DateComplication*)abstract_complication_to_helper(abs_complication, COMPLICATION_TYPE_DATE))
#define abstract_complication_to_weather(abs_complication) \
	((WeatherComplication*)abstract_complication_to_helper(abs_complication, COMPLICATION_TYPE_WEATHER))
#define abstract_complication_from_battery(abs_complication, complication) \
	abstract_complication_from_helper(abs_complication, complication, COMPLICATION_TYPE_BATTERY)
#define abstract_complication_from_date(abs_complication, complication) \
	abstract_complication_from_helper(abs_complication, complication, COMPLICATION_TYPE_DATE)
#define abstract_complication_from_weather(abs_complication, complication) \
	abstract_complication_from_helper(abs_complication, complication, COMPLICATION_TYPE_WEATHER)

#endif
