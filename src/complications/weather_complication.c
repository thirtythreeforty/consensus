#include "complication_common.h"

#include "common.h"

#include "constants.h"

typedef struct {
	WeatherData current_weather;

	GDrawCommandImage *icon;
	GPoint icon_shift;

	AppTimer *refresh_timer;

	WeatherData animation_weather;
	int32_t animation_temp_angle;
	int32_t animation_humidity_angle;
	bool animating;
} WeatherComplicationData;

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

static void weather_complication_update(Layer *layer, GContext *ctx)
{
	static const int HALF_MAX_ANGLE = TRIG_MAX_ANGLE / 2;

	WeatherComplicationData *data = layer_get_data(layer);

	// Always make the temp and angle show at least a little so the user is
	// aware that the display is two-sided
	int32_t humidity_angle = data->animating
		? data->animation_humidity_angle
		: weather_complication_humidity_angle(data->current_weather.humidity);
	int32_t temp_angle = data->animating
		? data->animation_temp_angle
		: weather_complication_temp_angle(data->current_weather.temp_c);

	if(!data->animating) {
		humidity_angle = CLAMP(HALF_MAX_ANGLE / 90, humidity_angle, HALF_MAX_ANGLE);
		temp_angle = CLAMP(HALF_MAX_ANGLE / 90, temp_angle, HALF_MAX_ANGLE);
	}

	if(!data->current_weather.valid) {
		humidity_angle = 0;
		temp_angle = 0;
	}

	base_complication_update_2(layer, ctx,
	                           GColorBlue, humidity_angle,
	                           GColorRed, temp_angle);

	// Draw icon (loaded in weather_complication_weather_changed).
	gdraw_command_image_draw(ctx, data->icon, data->icon_shift);
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
		MAX(slight_future, data->current_weather.time_updated + refresh_interval);

	const uint32_t delay_ms = (next_refresh_time - now) * 1000;

	app_timer_register(delay_ms, weather_complication_request_refresh, data);
}

WeatherComplication* weather_complication_create(GRect frame, const WeatherData *wdata)
{
	Layer *layer = layer_create_with_data(frame, sizeof(WeatherComplicationData));
	layer_set_update_proc(layer, weather_complication_update);

	WeatherComplicationData *data = layer_get_data(layer);
	data->icon = NULL;
	data->refresh_timer = NULL;
	data->animating = true;
	data->animation_temp_angle = 0;
	data->animation_humidity_angle = 0;
	weather_complication_weather_changed((WeatherComplication*)layer, wdata);

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

	data->current_weather = *new_weather;

	// Set the new icon
	free(data->icon);
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
		else {
			data->icon = NULL;
		}
	}
	else {
		data->icon = NULL;
	}

	weather_complication_schedule_refresh(data);

	layer_mark_dirty(layer);
}

static void weather_complication_spinup_animation_update(Animation *anim, AnimationProgress progress)
{
	WeatherComplication *complication = animation_get_context(anim);
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->animation_temp_angle = weather_complication_temp_angle(data->animation_weather.temp_c * progress / ANIMATION_NORMALIZED_MAX);
	data->animation_humidity_angle = weather_complication_humidity_angle(data->animation_weather.humidity * progress / ANIMATION_NORMALIZED_MAX);

	layer_mark_dirty(layer);
}

static void weather_complication_spinup_animation_started(Animation *anim, void *context)
{
	WeatherComplication *complication = context;
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->animating = true;
	data->animation_weather = data->current_weather;
	layer_mark_dirty(layer);
}

static void weather_complication_spinup_animation_stopped(Animation *anim, bool finished, void *context)
{
	WeatherComplication *complication = context;
	Layer *layer = weather_complication_get_layer(complication);
	WeatherComplicationData *data = layer_get_data(layer);

	data->animating = false;
	layer_mark_dirty(layer);
}

Animation* weather_complication_animate_in(WeatherComplication *complication)
{
	static const AnimationImplementation weather_spinup_anim_impl = {
		.update = weather_complication_spinup_animation_update
	};
	static const AnimationHandlers weather_spinup_anim_handlers = {
		.started = weather_complication_spinup_animation_started,
		.stopped = weather_complication_spinup_animation_stopped
	};

	return base_complication_animate_in(&weather_spinup_anim_impl,
	                                    &weather_spinup_anim_handlers,
	                                    complication);
}

