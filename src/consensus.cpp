#include <algorithm>
#include <array>
#include <memory>

extern "C" {
#include <pebble.h>
}

#include "common.h"
#include "constants.h"

#include "preferences.h"

#include "complications/complication.h"
#include "face_layer.h"

static Window *window = NULL;
static FaceLayer *face_layer = NULL;
static Layer *background_layer = NULL;
static std::array<AbstractComplication, 3> complications;

static enum {
	WAS_CONNECTED_INIT = 0,
	WAS_CONNECTED_FALSE,
	WAS_CONNECTED_TRUE
} was_connected = WAS_CONNECTED_INIT;
static BitmapLayer *no_bluetooth_layer = NULL;

static GDrawCommandImage *ticks_image = NULL;
static GBitmap *no_bluetooth_image = NULL;

using std::begin;
using std::end;

template<typename C, typename F>
void complication_do(const F& f)
{
	std::for_each(begin(complications), end(complications), [&](auto& c) {
		auto downcast_complication = c.template downcast<C>();
		if(downcast_complication) {
			f(*downcast_complication);
		}
	});
}

void on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	if(face_layer) {
		face_layer->set_time(tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);
	}

	complication_do<DateComplication>([&](auto& c) {
		c.time_changed(tick_time);
	});

	// Vibrate once on the hour and twice at noon.
	if(should_vibrate_on_hour() &&
	   tick_time->tm_min == 0 && tick_time->tm_sec == 0) {
		static const uint32_t vibe_pattern[] = {100, 250, 100};
		VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = static_cast<unsigned int>(tick_time->tm_hour % 12 == 0 ? 3 : 1)
		};

		vibes_enqueue_custom_pattern(vibe);
	}
}

void on_tap(AccelAxisType axis, int32_t direction)
{
	if(axis == ACCEL_AXIS_Z) {
		light_enable_interaction();
	}
}

void on_battery_state_change(BatteryChargeState charge)
{
	complication_do<BatteryComplication>([&](auto& c){
		c.state_changed(&charge);
	});
}

void on_connection_change(bool connected)
{
	layer_set_hidden(bitmap_layer_get_layer(no_bluetooth_layer),
	                 connected || should_hide_no_bluetooth());

	const bool became_disconnected = was_connected == WAS_CONNECTED_TRUE && !connected;

	if(became_disconnected && should_vibrate_on_disconnect()) {
		static const uint32_t vibe_pattern[] = {200, 250, 200, 250, 800};
		static const VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = NELEM(vibe_pattern)
		};

		vibes_enqueue_custom_pattern(vibe);
	}

	was_connected = connected ? WAS_CONNECTED_TRUE : WAS_CONNECTED_FALSE;
}

void ignore_connection_change(bool connected)
{
}

static void update_connection_now()
{
	on_connection_change(connection_service_peek_pebble_app_connection());
}

static void update_time_now()
{
	time_t abs_time = time(NULL);
	struct tm *tick_time = localtime(&abs_time);
	on_tick(tick_time, MINUTE_UNIT);
}

static TimeUnits update_time_interval(bool show_sec)
{
	return show_sec ? SECOND_UNIT : MINUTE_UNIT;
}

static void on_preferences_in(DictionaryIterator *iterator)
{
	parse_preferences(iterator);

	const bool show_second = should_show_second();
	const TimeUnits units = update_time_interval(show_second);
	tick_timer_service_subscribe(units, on_tick);
	face_layer->set_show_second(show_second);
	update_time_now();

	update_connection_now();
}

void on_appmessage_in(DictionaryIterator *iterator, void *context)
{
	WeatherData wdata;
	weather_from_appmessage(iterator, &wdata);
	if(wdata.valid) {
		complication_do<WeatherComplication>([&](auto& c) {
			c.weather_changed(wdata);
		});
		weather_to_persist(&wdata);
	}

	on_preferences_in(iterator);
}

void on_appmessage_in_dropped(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped (reason %i)!", reason);
}

static void update_background(Layer *layer, GContext *ctx)
{
	GRect rect = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, rect, 0, GCornerNone);

	gdraw_command_image_draw(ctx, ticks_image, GPointZero);
}

static void init_layers(void)
{
	GRect size = layer_get_bounds(window_get_root_layer(window));
	GPoint center = grect_center_point(&size);

	ticks_image = gdraw_command_image_create_with_resource(RESOURCE_ID_TICKS);
	no_bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_NO_BLUETOOTH);

	background_layer = layer_create(size);
	layer_set_update_proc(background_layer, update_background);
	layer_add_child(window_get_root_layer(window), background_layer);

	const int16_t complication_size = 51;
	const int16_t complication_offset_x = PBL_IF_ROUND_ELSE(15, 10);
	const int16_t complication_offset_y = 15;

	const GRect bluetooth_image_size = gbitmap_get_bounds(no_bluetooth_image);
	const GRect bluetooth_layer_location =
		GRect((int16_t)(center.x - bluetooth_image_size.size.w / 2),
		      (int16_t)(center.y - complication_offset_y - complication_size / 2 - bluetooth_image_size.size.h / 2),
		      (int16_t)bluetooth_image_size.size.w,
		      (int16_t)bluetooth_image_size.size.h);
	no_bluetooth_layer = bitmap_layer_create(bluetooth_layer_location);
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);
	bitmap_layer_set_compositing_mode(no_bluetooth_layer, GCompOpSet);

	// Immediately hide or show the icon
	update_connection_now();
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(no_bluetooth_layer));

	const GRect left_complication_position =
		GRect((int16_t)(center.x - complication_size - complication_offset_x),
		      (int16_t)(center.y - complication_size / 2),
		      (int16_t)complication_size,
		      (int16_t)complication_size);
	complications[0] = AbstractComplication::create(left_complication_type(), left_complication_position);

	const GRect right_complication_position =
		GRect((int16_t)(center.x + complication_offset_x),
		      (int16_t)(center.y - complication_size / 2),
		      (int16_t)complication_size,
		      (int16_t)complication_size);
	complications[1] = AbstractComplication::create(right_complication_type(), right_complication_position);

	const GRect bottom_complication_position =
		GRect((int16_t)(center.x - complication_size / 2),
		      (int16_t)(center.y + complication_offset_y),
		      (int16_t)complication_size,
		      (int16_t)complication_size);
	complications[2] = AbstractComplication::create(bottom_complication_type(), bottom_complication_position);

	// Initialize the complications
	for(auto& complication: complications) {
		if(complication.valid()) {
			layer_add_child(window_get_root_layer(window), static_cast<Complication&>(complication));
		}
	}

	const BatteryChargeState charge_state = battery_state_service_peek();
	on_battery_state_change(charge_state);

	WeatherData wdata;
	weather_from_persist(&wdata);
	complication_do<WeatherComplication>([&](auto& c) {
		c.weather_changed(wdata);
	});

	complication_do<DateComplication>([](auto& c) {
		animation_schedule(c.animate_in());
	});

	face_layer = new FaceLayer(size);
	face_layer->set_colors(GColorVeryLightBlue, GColorPictonBlue, GColorRed);
	face_layer->set_show_second(should_show_second());
	layer_add_child(window_get_root_layer(window), *face_layer);
	animation_schedule(face_layer->animate_in(true, true));
}

static void deinit_layers(void)
{
	delete face_layer;
	for(unsigned int i = 0; i < NELEM(complications); ++i) {
		complications[i].destroy();
	}
	bitmap_layer_destroy(no_bluetooth_layer);
	gbitmap_destroy(no_bluetooth_image);
	layer_destroy(background_layer);
	free(ticks_image);
}

void main_window_load(Window *window)
{
	init_layers();
}

void main_window_unload(Window *window)
{
	deinit_layers();
}

static void init(void)
{
	window = window_create();
	static const WindowHandlers h = {
		main_window_load,
		NULL,
		NULL,
		main_window_unload
	};
	window_set_window_handlers(window, h);
	window_stack_push(window, true);

	const TimeUnits units = update_time_interval(should_show_second());
	tick_timer_service_subscribe(units, on_tick);

	battery_state_service_subscribe(on_battery_state_change);

	accel_tap_service_subscribe(on_tap);

	static const ConnectionHandlers conn_handlers = {
		.pebble_app_connection_handler = on_connection_change,
		.pebblekit_connection_handler = ignore_connection_change,
	};
	connection_service_subscribe(conn_handlers);

	app_message_register_inbox_received(on_appmessage_in);
	app_message_register_inbox_dropped(on_appmessage_in_dropped);
	app_message_open(128, 64);

	update_time_now();
}

static void deinit(void)
{
	animation_unschedule_all();
	app_message_deregister_callbacks();
	connection_service_unsubscribe();
	accel_tap_service_unsubscribe();
	battery_state_service_unsubscribe();
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
