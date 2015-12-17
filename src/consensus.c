#include <pebble.h>
#include "common.h"

#include "face_layer.h"
#include "complications/complication.h"

static Window *window;
static FaceLayer *face_layer;
static Layer *black_layer;
static TextLayer *date_layer;
static BatteryComplication *battery_complication;

static void set_date(struct tm *tick_time)
{
	static char date_layer_text[8] = "";
	strftime(date_layer_text, NELEM(date_layer_text), "%a %d", tick_time);
	text_layer_set_text(date_layer, date_layer_text);
}

void on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	if(face_layer) {
		face_layer_set_time(face_layer, tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);
	}

	if(date_layer) {
		set_date(tick_time);
	}
}

void on_battery_state_change(BatteryChargeState charge)
{
	if(battery_complication) {
		battery_complication_state_changed(battery_complication, &charge);
	}
}

static void update_time()
{
	time_t abs_time = time(0);
	struct tm *tick_time = localtime(&abs_time);
	on_tick(tick_time, MINUTE_UNIT);
}

static void fill_black(Layer *layer, GContext *ctx)
{
	GRect rect = layer_get_bounds(layer);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, rect, 0, 0);
}

static void init_layers(void)
{
	GRect size = layer_get_bounds(window_get_root_layer(window));
	GPoint center = grect_center_point(&size);

	black_layer = layer_create(size);
	layer_set_update_proc(black_layer, fill_black);
	layer_add_child(window_get_root_layer(window), black_layer);

	BatteryChargeState charge_state = battery_state_service_peek();
	const unsigned int complication_size = 51;
	GRect complication_position = {{ .x = center.x - complication_size - 20, .y = center.y - complication_size / 2 },
	                               { .h = complication_size, .w = complication_size }};
	battery_complication = battery_complication_create(complication_position, &charge_state);
	layer_add_child(window_get_root_layer(window), battery_complication_get_layer(battery_complication));
	battery_complication_animate_in(battery_complication);

	// As far as I can tell this is just a magic number.  5px margins per side?
	static const unsigned int date_margins = 10;
	static const unsigned int date_font_height = 24;
	static const unsigned int date_height = date_font_height + date_margins;
#ifdef PBL_ROUND
	date_layer = text_layer_create(GRect(center.x, center.y - date_height / 2,
	                                     center.x - 8, date_height));
#else
	date_layer = text_layer_create(GRect(0, center.y + 24,
	                                     size.size.w, date_height));
#endif
	text_layer_set_text_alignment(date_layer, PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentCenter));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorLightGray);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

	face_layer = face_layer_create(size);
	face_layer_set_colors(face_layer, GColorCobaltBlue, GColorPictonBlue, GColorRed);
	layer_add_child(window_get_root_layer(window), face_layer);
	face_layer_animate_in(face_layer, true, true);
}

static void deinit_layers(void)
{
	text_layer_destroy(date_layer);
	face_layer_destroy(face_layer);
	battery_complication_destroy(battery_complication);
	layer_destroy(black_layer);
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

	tick_timer_service_subscribe(SECOND_UNIT, on_tick);

	battery_state_service_subscribe(on_battery_state_change);

	update_time();
}

static void deinit(void)
{
	animation_unschedule_all();
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	window_destroy(window);
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
