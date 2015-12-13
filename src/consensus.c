#include <pebble.h>

#include "face_layer.h"

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

static Window *window;
static FaceLayer *face_layer;

void on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	face_layer_set_time(face_layer, tick_time);
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
	Layer *black_layer = layer_create(GRect(0, 0, 144, 168));
	layer_set_update_proc(black_layer, fill_black);
	layer_add_child(window_get_root_layer(window), black_layer);

	face_layer = face_layer_create();
	face_layer_set_colors(face_layer, GColorCobaltBlue, GColorPictonBlue);
	layer_add_child(window_get_root_layer(window), face_layer);
}

static void deinit_layers(void)
{
	layer_remove_from_parent(face_layer);
	face_layer_destroy(face_layer);
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

	update_time();
}

static void deinit(void)
{
	tick_timer_service_unsubscribe();
	window_destroy(window);
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
