#ifndef COMPLICATION_H
#define COMPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} // extern "C"

template<typename T> constexpr uint8_t complication_type_map;
template<> constexpr uint8_t complication_type_map<void> = 0;
template<> constexpr uint8_t complication_type_map<BatteryComplication> = 1;
template<> constexpr uint8_t complication_type_map<DateComplication> = 2;
template<> constexpr uint8_t complication_type_map<WeatherComplication> = 3;

class AbstractComplication
{
	void *complication;
	uint8_t type;

public:
	AbstractComplication() : type(complication_type_map<void>) {}

	template<typename T>
	AbstractComplication(T *ptr)
		: complication(ptr)
		, type(complication_type_map<T>)
	{}

	template<typename T>
	auto downcast() -> T* {
		if(type == complication_type_map<T>) {
			return static_cast<T*>(complication);
		}
		else {
			return nullptr;
		}
	}

	template<typename T>
	static auto from(T *ptr) -> AbstractComplication {
		return AbstractComplication{ptr};
	}

	void destroy() {
		switch(type) {
		case complication_type_map<void>:
			break;
		case complication_type_map<BatteryComplication>:
			battery_complication_destroy(downcast<BatteryComplication>());
			break;
		case complication_type_map<DateComplication>:
			date_complication_destroy(downcast<DateComplication>());
			break;
		case complication_type_map<WeatherComplication>:
			weather_complication_destroy(downcast<WeatherComplication>());
			break;
		}
		type = complication_type_map<void>;
	}
};

#include "boulder.h"

class Complication
{
	Boulder::Layer blayer;

public:
	virtual ~Complication();

	const Boulder::Layer& layer() const { return blayer; }
};

class BatteryComplication: Complication
{
	bool animating;

	BatteryChargeState requested_state;

	uint32_t angle;
	GDrawCommandImage *icon;

public:
	BatteryComplication();
	virtual ~BatteryComplication();
};
#endif

#endif
