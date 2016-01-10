#include "complication_common.h"

#include "common.h"
#include "constants.h"

typedef struct {
	int32_t temp_angle;
	int32_t humidity_angle;
} WeatherAngles;

typedef struct {
	WeatherAngles angles;
	GDrawCommandImage *icon;
	GPoint icon_shift;

	WeatherData requested_weather;
	bool animating;

	AppTimer *refresh_timer;
} WeatherComplicationData;

static void weather_complication_animate_to(WeatherComplication *complication, WeatherAngles angles);

void weather_from_appmessage(DictionaryIterator *iterator, WeatherData *wdata)
{
	Tuple *temp_tuple = dict_find(iterator, KEY_WEATHER_TEMP_C);
	Tuple *humidity_tuple = dict_find(iterator, KEY_WEATHER_HUMIDITY);
	Tuple *icon_tuple = dict_find(iterator, KEY_WEATHER_ICON);

	if(temp_tuple && humidity_tuple && icon_tuple) {
		int temp = temp_tuple->value->int32;
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
		.temp_c = persist_read_int(PERSIST_WEATHER_TEMP_C),
		.humidity = persist_read_int(PERSIST_WEATHER_HUMIDITY),
		.icon = persist_read_int(PERSIST_WEATHER_ICON)
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

static WeatherAngles weather_complication_angles(const WeatherData *wdata)
{
	static const int HALF_MAX_ANGLE = TRIG_MAX_ANGLE / 2;

	if(wdata->valid) {
		// Always make the temp and angle show at least a little so the user is
		// aware that the display is two-sided
		return (WeatherAngles) {
			.temp_angle = CLAMP(HALF_MAX_ANGLE / 90,
			                    weather_complication_temp_angle(wdata->temp_c),
			                    HALF_MAX_ANGLE),
			.humidity_angle = CLAMP(HALF_MAX_ANGLE / 90,
			                        weather_complication_humidity_angle(wdata->humidity),
			                        HALF_MAX_ANGLE)
		};
	}
	else {
		return (WeatherAngles) { 0, 0 };
	}
}

static void weather_complication_update(Layer *layer, GContext *ctx)
{
	WeatherComplicationData *data = layer_get_data(layer);

	base_complication_update_2(layer, ctx,
	                           GColorBlue, data->angles.humidity_angle,
	                           GColorRed, data->angles.temp_angle);

	// Draw icon (loaded in weather_complication_weather_changed).
	if(data->icon) {
		gdraw_command_image_draw(ctx, data->icon, data->icon_shift);
	}
}

static void weather_complication_request_refresh(void *ptr)
{
	DictionaryIterator *iter;
	static const uint8_t value = 0;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_WEATHER_REQUEST, &value, sizeof value, false);
	app_message_outbox_send();
}

static void weather_complication_cancel_timer(WeatherComplicationData *data)
{
	if(data->refresh_timer) {
		app_timer_cancel(data->refresh_timer);
		data->refresh_timer = NULL;
	}
}

static void weather_complication_schedule_refresh(WeatherComplicationData *data)
{
	weather_complication_cancel_timer(data);

	time_t now = time(NULL);

	static const unsigned int refresh_interval = 60 * 60 * 3;
	const unsigned int slight_future = now + 10;

	const unsigned int next_refresh_time =
		MAX(slight_future, data->requested_weather.time_updated + refresh_interval);

	const uint32_t delay_ms = (next_refresh_time - now) * 1000;

	app_timer_register(delay_ms, weather_complication_request_refresh, data);
}

WeatherComplication* weather_complication_create(GRect frame)
{
	Layer *layer = layer_create_with_data(frame, sizeof(WeatherComplicationData));
	layer_set_update_proc(layer, weather_complication_update);

	WeatherComplicationData *data = layer_get_data(layer);
	*data = (WeatherComplicationData) {
		// Some fields omitted purposefully; they will be updated when the
		// weather data is received
		.angles = { 0, 0 },
		.icon = NULL,

		.animating = false,
		.refresh_timer = NULL,
	};

	return (WeatherComplication*)layer;
}

void weather_complication_destroy(WeatherComplication *complication)
{
	Layer *layer = weather_complication_get_layer(complication);

	WeatherComplicationData *data = layer_get_data(layer);
	free(data->icon);
	weather_complication_cancel_timer(data);

	layer_destroy(layer);
}

Layer* weather_complication_get_layer(WeatherComplication *complication)
{
	return (Layer*)complication;
}

static bool weather_complication_change_icon_color(GDrawCommand *command, uint32_t index, void *context)
{
	gdraw_command_set_fill_color(command, GColorClear);
	gdraw_command_set_stroke_color(command, GColorDarkGray);

	return true;
}

void weather_complication_weather_changed(WeatherComplication *complication, const WeatherData *new_weather)
{
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->requested_weather = *new_weather;

	// Animate to the new weather
	if(!data->animating) {
		WeatherAngles angles = weather_complication_angles(new_weather);
		weather_complication_animate_to(complication, angles);
	}

	// Set the new icon
	free(data->icon);
	data->icon = NULL;

	if(new_weather->valid) {
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
		if(new_weather->icon <= NELEM(weather_icons)) {
			const uint32_t resource = weather_icons[new_weather->icon];

			data->icon = gdraw_command_image_create_with_resource(resource);

			// Change the icon color.  This is gonna get ugly...
			gdraw_command_list_iterate(gdraw_command_image_get_command_list(data->icon),
			                           weather_complication_change_icon_color, NULL);

			// Need to shift it over to account for its size.
			const GSize icon_size = gdraw_command_image_get_bounds_size(data->icon);
			const GRect bounds = layer_get_bounds(layer);
			data->icon_shift = (GPoint) {
				.x = bounds.size.w / 2 - icon_size.w / 2,
				.y = bounds.size.h / 2 - icon_size.h / 2
			};
		}
	}

	weather_complication_schedule_refresh(data);

	layer_mark_dirty(layer);
}

static void weather_complication_spinup_animation_started(Animation *anim, void *context)
{
	WeatherComplication *complication = context;
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->animating = true;
}

static void weather_complication_spinup_animation_stopped(Animation *anim, bool finished, void *context)
{
	WeatherComplication *complication = context;
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->animating = false;

	const WeatherAngles requested_angles = weather_complication_angles(&data->requested_weather);

	if(requested_angles.temp_angle != data->angles.temp_angle ||
	   requested_angles.humidity_angle != data->angles.humidity_angle) {
		weather_complication_animate_to(complication, requested_angles);
	}
}

static WeatherAngles weather_complication_get_angles(void *subject)
{
	WeatherComplication *complication = subject;
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	return data->angles;
}

typedef void (*WeatherAnglesSetter)(void*, WeatherAngles);

static void weather_complication_set_angles(void *subject, WeatherAngles angles)
{
	WeatherComplication *complication = subject;
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->angles = angles;

	layer_mark_dirty(layer);
}

static inline int32_t int32_interpolate(uint32_t distance, int32_t from, int32_t to)
{
	return from + ((distance * (to - from)) / ANIMATION_NORMALIZED_MAX);
}

static void property_animation_update_weather_angles(PropertyAnimation *property_animation, const uint32_t distance)
{
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
	WeatherAnglesSetter setter = (WeatherAnglesSetter)impl->accessors.setter.uint32;
	void *subject;
	property_animation_subject(property_animation, &subject, false);
	if(setter && subject) {
		setter(subject, interpolated);
	}
}

void weather_complication_animate_to(WeatherComplication *complication, WeatherAngles requested_angles)
{
	static const PropertyAnimationImplementation weather_spin_impl = {
		.base = {
			.update = (AnimationUpdateImplementation)property_animation_update_weather_angles
		},
		.accessors = {
			.setter = { .uint32 = (UInt32Setter)weather_complication_set_angles },
			.getter = { .uint32 = (UInt32Getter)weather_complication_get_angles }
		}
	};
	static const AnimationHandlers weather_spinup_anim_handlers = {
		.started = weather_complication_spinup_animation_started,
		.stopped = weather_complication_spinup_animation_stopped
	};

	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	PropertyAnimation *property_anim = property_animation_create(
		&weather_spin_impl,
		complication,
		NULL, NULL
	);
	property_animation_from(property_anim, &data->angles, sizeof(WeatherAngles), true);
	property_animation_to(property_anim, &requested_angles, sizeof(requested_angles), true);

	Animation *anim = property_animation_get_animation(property_anim);
	animation_schedule(base_complication_setup_animation(anim, &weather_spinup_anim_handlers, complication));
}

