#include <pebble.h>
#include "common.h"

#include "face_layer.h"
#include "complications/complication.h"

static Window *window;
static FaceLayer *face_layer;
static Layer *black_layer;
static DateComplication *date_complication;
static BitmapLayer *no_bluetooth_layer;
static BatteryComplication *battery_complication;

static GDrawCommandImage *ticks_image;
static GBitmap *no_bluetooth_image;

void on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	if(face_layer) {
		face_layer_set_time(face_layer, tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);
	}

	if(date_complication) {
		date_complication_time_changed(date_complication, tick_time);
	}

	// Vibrate once on the hour and twice at noon.
	if(tick_time->tm_min == 0 && tick_time->tm_sec == 0) {
		static const uint32_t vibe_pattern[] = {100, 250, 100};
		VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = tick_time->tm_hour % 12 == 0 ? 3 : 1
		};

		vibes_enqueue_custom_pattern(vibe);
	}
}

void on_tap(AccelAxisType axis, int32_t direction)
{
	APP_DEBUG("Handling acceleration on axis %i", axis);
	if(axis == ACCEL_AXIS_Z) {
		light_enable_interaction();
	}
}

void on_battery_state_change(BatteryChargeState charge)
{
	if(battery_complication) {
		battery_complication_state_changed(battery_complication, &charge);
	}
}

void on_connection_change(bool connected)
{
	layer_set_hidden(bitmap_layer_get_layer(no_bluetooth_layer), connected);
}

void ignore_connection_change(bool connected)
{
}

static void fill_black(Layer *layer, GContext *ctx)
{
	GRect rect = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, rect, 0, 0);

	gdraw_command_image_draw(ctx, ticks_image, GPointZero);
}

static void init_layers(void)
{
	GRect size = layer_get_bounds(window_get_root_layer(window));
	GPoint center = grect_center_point(&size);

	ticks_image = gdraw_command_image_create_with_resource(RESOURCE_ID_TICKS);
	no_bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_NO_BLUETOOTH);

	black_layer = layer_create(size);
	layer_set_update_proc(black_layer, fill_black);
	layer_add_child(window_get_root_layer(window), black_layer);

	const unsigned int complication_size = 51;
	const unsigned int complication_offset_x = PBL_IF_ROUND_ELSE(15, 10);
	const unsigned int complication_offset_y = 15;

	const GRect bluetooth_image_size = gbitmap_get_bounds(no_bluetooth_image);
	const GRect bluetooth_layer_location =
		{{ .x = center.x - bluetooth_image_size.size.w / 2,
		   .y = center.y - complication_offset_y - complication_size / 2 - bluetooth_image_size.size.h / 2 },
		 { .h = bluetooth_image_size.size.h,
		   .w = bluetooth_image_size.size.w }};
	no_bluetooth_layer = bitmap_layer_create(bluetooth_layer_location);
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);
	bitmap_layer_set_compositing_mode(no_bluetooth_layer, GCompOpSet);
	// Immediately hide or show the icon
	on_connection_change(connection_service_peek_pebble_app_connection());
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(no_bluetooth_layer));

	BatteryChargeState charge_state = battery_state_service_peek();
	const GRect battery_complication_position =
		{{ .x = center.x - complication_size - complication_offset_x,
		   .y = center.y - complication_size / 2 },
		 { .h = complication_size,
		   .w = complication_size }};
	battery_complication = battery_complication_create(battery_complication_position, &charge_state);
	layer_add_child(window_get_root_layer(window), battery_complication_get_layer(battery_complication));
	battery_complication_animate_in(battery_complication);

	const GRect date_complication_position =
		{{ .x = center.x + complication_offset_x,
		   .y = center.y - complication_size / 2 },
		 { .h = complication_size,
		   .w = complication_size }};
	date_complication = date_complication_create(date_complication_position);
	layer_add_child(window_get_root_layer(window), date_complication_get_layer(date_complication));
	date_complication_animate_in(date_complication);

	face_layer = face_layer_create(size);
	face_layer_set_colors(face_layer, GColorCobaltBlue, GColorPictonBlue, GColorRed);
	face_layer_set_show_second(face_layer, false);
	layer_add_child(window_get_root_layer(window), face_layer);
	face_layer_animate_in(face_layer, true, true);
}

static void deinit_layers(void)
{
	face_layer_destroy(face_layer);
	date_complication_destroy(date_complication);
	battery_complication_destroy(battery_complication);
	bitmap_layer_destroy(no_bluetooth_layer);
	gbitmap_destroy(no_bluetooth_image);
	layer_destroy(black_layer);
	free(ticks_image);
}

static void main_window_load(Window *window)
{
	init_layers();
}

static void main_window_unload(Window *window)
{
	deinit_layers();
}

static void init(void)
{
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload,
	});
	window_stack_push(window, true);

	tick_timer_service_subscribe(MINUTE_UNIT, on_tick);

	battery_state_service_subscribe(on_battery_state_change);

	accel_tap_service_subscribe(on_tap);

	static const ConnectionHandlers conn_handlers = {
		.pebble_app_connection_handler = on_connection_change,
		.pebblekit_connection_handler = ignore_connection_change,
	};
	connection_service_subscribe(conn_handlers);

	time_t abs_time = time(0);
	struct tm *tick_time = localtime(&abs_time);
	on_tick(tick_time, MINUTE_UNIT);
}

static void deinit(void)
{
	animation_unschedule_all();
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
