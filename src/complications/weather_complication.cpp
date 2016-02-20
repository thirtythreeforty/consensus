#include "complication.h"

#include "common.h"
#include "constants.h"
#include "themes.h"

const char* WeatherComplication::deg_format = "%i\u00B0";
const char* WeatherComplication::relhum_format = "%i";
const char* WeatherComplication::empty_format = "";

using std::experimental::nullopt;
std::experimental::optional<Boulder::AppTimer> WeatherComplication::refresh_timer
	= nullopt;

void weather_from_appmessage(DictionaryIterator *iterator, WeatherData *wdata)
{
	Tuple *temp_tuple = dict_find(iterator, KEY_WEATHER_TEMP_C);
	Tuple *humidity_tuple = dict_find(iterator, KEY_WEATHER_HUMIDITY);
	Tuple *icon_tuple = dict_find(iterator, KEY_WEATHER_ICON);

	if(temp_tuple && humidity_tuple && icon_tuple) {
		int8_t temp = temp_tuple->value->int8;
		uint8_t humidity = humidity_tuple->value->uint8;
		uint8_t icon = icon_tuple->value->uint8;

		*wdata = (WeatherData) {
			.valid = true,
			.time_updated = time(NULL),
			.temp_c = temp,
			.humidity = humidity,
			.icon = icon
		};
	}
	else {
		// If some elements are present (but not all), there is an error
		if(temp_tuple || humidity_tuple || icon_tuple) {
			APP_LOG(APP_LOG_LEVEL_ERROR,
			        "Bad weather data received (%p, %p, %p)",
			        temp_tuple, humidity_tuple, icon_tuple);
		}

		wdata->valid = false;
	}
}

void weather_from_persist(WeatherData *wdata)
{
	if(!(persist_exists(PERSIST_WEATHER_TIME_UPDATED) &&
	     persist_exists(PERSIST_WEATHER_TEMP_C) &&
	     persist_exists(PERSIST_WEATHER_HUMIDITY) &&
	     persist_exists(PERSIST_WEATHER_ICON))) {
		wdata->valid = false;
		return;
	}

	*wdata = (WeatherData) {
		.valid = true,
		.time_updated = persist_read_int(PERSIST_WEATHER_TIME_UPDATED),
		.temp_c = static_cast<int8_t>(persist_read_int(PERSIST_WEATHER_TEMP_C)),
		.humidity = static_cast<uint8_t>(persist_read_int(PERSIST_WEATHER_HUMIDITY)),
		.icon = static_cast<uint8_t>(persist_read_int(PERSIST_WEATHER_ICON))
	};
}

void weather_to_persist(const WeatherData *data)
{
	if(data->valid) {
		persist_write_int(PERSIST_WEATHER_TIME_UPDATED, data->time_updated);
		persist_write_int(PERSIST_WEATHER_TEMP_C, data->temp_c);
		persist_write_int(PERSIST_WEATHER_HUMIDITY, data->humidity);
		persist_write_int(PERSIST_WEATHER_ICON, data->icon);
	}
	else {
		persist_delete(PERSIST_WEATHER_TIME_UPDATED);
		persist_delete(PERSIST_WEATHER_TEMP_C);
		persist_delete(PERSIST_WEATHER_HUMIDITY);
		persist_delete(PERSIST_WEATHER_ICON);
	}
}

static int weather_complication_temp_angle(int8_t temp_c)
{
	static const int HALF_MAX_ANGLE = TRIG_MAX_ANGLE / 2;

	static const int min_temp = -5;
	static const int max_temp = 43;

	return (temp_c - min_temp) * HALF_MAX_ANGLE / (max_temp - min_temp);
}

static int weather_complication_humidity_angle(uint8_t humidity)
{
	static const int HALF_MAX_ANGLE = TRIG_MAX_ANGLE / 2;

	return humidity * HALF_MAX_ANGLE / 100;
}

auto WeatherComplication::compute_angles(const WeatherData &wdata) -> WeatherAngles
{
	static const int HALF_MAX_ANGLE = TRIG_MAX_ANGLE / 2;

	if(wdata.valid) {
		// Always make the temp and angle show at least a little so the user is
		// aware that the display is two-sided
		return (WeatherAngles) {
			.temp_angle = clamp(static_cast<int>(HALF_MAX_ANGLE / 90),
			                    weather_complication_temp_angle(wdata.temp_c),
			                    HALF_MAX_ANGLE),
			.humidity_angle = clamp(static_cast<int>(HALF_MAX_ANGLE / 90),
			                        weather_complication_humidity_angle(wdata.humidity),
			                        HALF_MAX_ANGLE)
		};
	}
	else {
		return (WeatherAngles) { 0, 0 };
	}
}

void WeatherComplication::request_refresh(void*)
{
	DictionaryIterator *iter;
	static const uint8_t value = 0;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_WEATHER_REQUEST, &value, sizeof value, false);
	app_message_outbox_send();

	// TODO this causes an undesired "timer doesn't exist" message (which is true)
	// because the timer deletes itself after it fires.  Boulder::AppTimer needs logic
	// to handle the timer firing.
	refresh_timer = nullopt;
}

void WeatherComplication::schedule_refresh(time_t last_refresh_time)
{
	if(!refresh_timer) {
		time_t now = time(nullptr);

		static const time_t refresh_interval = 60 * 60 * 2;
		const time_t slight_future = now + 10;

		const unsigned int next_refresh_time =
			std::max(slight_future, last_refresh_time + refresh_interval);

		const uint32_t delay_ms = (next_refresh_time - now) * 1000;

		refresh_timer.emplace(delay_ms, WeatherComplication::request_refresh, nullptr);
	}
}

WeatherComplication::WeatherComplication(GRect frame)
	: HighlightComplication2(frame)
{}

void WeatherComplication::weather_changed(const WeatherData &new_weather)
{
	auto angles = compute_angles(new_weather);
	set_angle(angles.humidity_angle);
	set_angle2(angles.temp_angle);

	reset_icon();

	switch(gadget_type) {
	case ICON:
		// Set the new icon
		if(new_weather.valid) {
			static const std::array<uint32_t, 9> weather_icons = {
				RESOURCE_ID_WEATHER_GENERIC,
				RESOURCE_ID_WEATHER_SUNNY,
				RESOURCE_ID_WEATHER_CLOUDY,
				RESOURCE_ID_WEATHER_PARTLY_CLOUDY,
				RESOURCE_ID_WEATHER_RAIN_LIGHT,
				RESOURCE_ID_WEATHER_RAIN_HEAVY,
				RESOURCE_ID_WEATHER_RAIN_AND_SNOW,
				RESOURCE_ID_WEATHER_SNOW_LIGHT,
				RESOURCE_ID_WEATHER_SNOW_HEAVY,
			};

			// Be sure and clamp the index so we're not accessing arbitrary memory.
			if(new_weather.icon <= weather_icons.size()) {
				const uint32_t resource = weather_icons[new_weather.icon];

				set_icon(resource);
			}
		}
		set_number_format(empty_format, 0);
		break;
	case TEMP_F:
		set_number_format(deg_format, (new_weather.temp_c * 18 + 320) / 10);
		break;
	case TEMP_C:
		set_number_format(deg_format, new_weather.temp_c);
		break;
	case RELHUM:
		set_number_format(relhum_format, new_weather.humidity);
		break;
	}

	schedule_refresh(new_weather.time_updated);

	mark_dirty();
}

void WeatherComplication::configure(const config_bundle_t& config)
{
	HighlightComplication2::configure(config);

	gadget_type = static_cast<GadgetType>(std::get<0>(config));
	mark_dirty();
}

GColor WeatherComplication::highlight_color() const
{
	return GColorBlue;
}

GColor WeatherComplication::highlight_color2() const
{
	return GColorRed;
}
