#include "complication_common.h"

#include "common.h"
#include "constants.h"

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

static int32_t weather_complication_temp_angle(int8_t temp_c)
{
	static const int HALF_MAX_ANGLE = TRIG_MAX_ANGLE / 2;

	static const int min_temp = -5;
	static const int max_temp = 43;

	return (temp_c - min_temp) * HALF_MAX_ANGLE / (max_temp - min_temp);
}

static int32_t weather_complication_humidity_angle(uint8_t humidity)
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
			.temp_angle = CLAMP(HALF_MAX_ANGLE / 90,
			                    weather_complication_temp_angle(wdata.temp_c),
			                    HALF_MAX_ANGLE),
			.humidity_angle = CLAMP(HALF_MAX_ANGLE / 90,
			                        weather_complication_humidity_angle(wdata.humidity),
			                        HALF_MAX_ANGLE)
		};
	}
	else {
		return (WeatherAngles) { 0, 0 };
	}
}

void WeatherComplication::update(GContext *ctx)
{
	base_complication_update_2(*this, ctx,
	                           GColorBlue, angles.humidity_angle,
	                           GColorRed, angles.temp_angle);

	// Draw icon (loaded in weather_complication_weather_changed).
	if(icon) {
		icon->draw(ctx, icon_shift);
	}
}

void WeatherComplication::request_refresh(void*)
{
	DictionaryIterator *iter;
	static const uint8_t value = 0;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_WEATHER_REQUEST, &value, sizeof value, false);
	app_message_outbox_send();
}

void WeatherComplication::schedule_refresh()
{
	time_t now = time(NULL);

	static const unsigned int refresh_interval = 60 * 60 * 2;
	const unsigned int slight_future = now + 10;

	const unsigned int next_refresh_time =
		MAX(slight_future, requested_weather.time_updated + refresh_interval);

	const uint32_t delay_ms = (next_refresh_time - now) * 1000;

	refresh_timer.emplace(delay_ms, WeatherComplication::request_refresh, nullptr);
}

WeatherComplication::WeatherComplication(GRect frame)
	: Complication(frame)
	, angles({0, 0})
	, animating(false)
{}

void WeatherComplication::weather_changed(const WeatherData &new_weather)
{
	requested_weather = new_weather;

	// Animate to the new weather
	if(!animating) {
		WeatherAngles angles = compute_angles(new_weather);
		animate_to(angles);
	}

	// Set the new icon
	icon = std::experimental::nullopt;

	if(new_weather.valid) {
		static const uint32_t weather_icons[] = {
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
		if(new_weather.icon <= NELEM(weather_icons)) {
			const uint32_t resource = weather_icons[new_weather.icon];

			icon.emplace(resource);

			// Change the icon color
			icon->iterate([](GDrawCommand *command, uint32_t) {
				gdraw_command_set_fill_color(command, GColorClear);
				gdraw_command_set_stroke_color(command, GColorDarkGray);
				return true;
			});

			// Need to shift it over to account for its size.
			const GSize icon_size = icon->get_bounds_size();
			const GRect bounds = this->get_bounds();
			icon_shift = {
				.x = static_cast<int16_t>(bounds.size.w / 2 - icon_size.w / 2),
				.y = static_cast<int16_t>(bounds.size.h / 2 - icon_size.h / 2)
			};
		}
	}

	schedule_refresh();

	mark_dirty();
}

void WeatherComplication::spinup_animation_started(Animation *anim, void *context)
{
	auto complication = static_cast<WeatherComplication*>(context);
	complication->animating = true;
}

void WeatherComplication::spinup_animation_stopped(Animation *anim, bool finished, void *context)
{
	auto complication = static_cast<WeatherComplication*>(context);
	complication->animating = false;

	const WeatherAngles requested_angles = compute_angles(complication->requested_weather);

	if(requested_angles.temp_angle != complication->angles.temp_angle ||
	   requested_angles.humidity_angle != complication->angles.humidity_angle) {
		complication->animate_to(requested_angles);
	}
}

auto WeatherComplication::get_angles(void *subject) -> WeatherAngles
{
	auto complication = static_cast<WeatherComplication*>(subject);
	return complication->angles;
}

void WeatherComplication::set_angles(void *subject, WeatherAngles angles)
{
	auto complication = static_cast<WeatherComplication*>(subject);
	complication->angles = angles;

	complication->mark_dirty();
}

static inline int32_t int32_interpolate(uint32_t distance, int32_t from, int32_t to)
{
	return from + (((int32_t)distance * (to - from)) / ANIMATION_NORMALIZED_MAX);
}

void property_animation_update_weather_angles(PropertyAnimation *property_animation, const uint32_t distance)
{
	using WeatherAngles = WeatherComplication::WeatherAngles;
	typedef void (*WeatherAnglesSetter)(void*, WeatherAngles);

	WeatherAngles from, to;

	property_animation_from(property_animation, &from, sizeof(from), false);
	property_animation_to(property_animation, &to, sizeof(to), false);

	WeatherAngles interpolated = {
		.temp_angle = int32_interpolate(distance, from.temp_angle, to.temp_angle),
		.humidity_angle = int32_interpolate(distance, from.humidity_angle, to.humidity_angle)
	};

	Animation *anim = property_animation_get_animation(property_animation);
	PropertyAnimationImplementation *impl =
		(PropertyAnimationImplementation*)animation_get_implementation(anim);
	auto setter = reinterpret_cast<WeatherAnglesSetter>(impl->accessors.setter.uint32);
	void *subject;
	property_animation_subject(property_animation, &subject, false);
	if(setter && subject) {
		setter(subject, interpolated);
	}
}

void WeatherComplication::animate_to(WeatherAngles requested_angles)
{
	static const PropertyAnimationImplementation weather_spin_impl = {
		.base = {
			NULL,
			(AnimationUpdateImplementation)property_animation_update_weather_angles,
			NULL
		},
		.accessors = {
			(Int16Setter)WeatherComplication::set_angles,
			(Int16Getter)WeatherComplication::get_angles
		}
	};
	static const AnimationHandlers weather_spinup_anim_handlers = {
		.started = WeatherComplication::spinup_animation_started,
		.stopped = WeatherComplication::spinup_animation_stopped
	};

	PropertyAnimation *property_anim = property_animation_create(
		&weather_spin_impl,
		this,
		NULL, NULL
	);
	property_animation_from(property_anim, &angles, sizeof(angles), true);
	property_animation_to(property_anim, &requested_angles, sizeof(requested_angles), true);

	Animation *anim = property_animation_get_animation(property_anim);
	animation_schedule(base_complication_setup_animation(anim, &weather_spinup_anim_handlers, this));
}

