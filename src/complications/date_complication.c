#include "complication_common.h"

#include "common.h"

typedef struct {
	bool animating;
	uint8_t anim_frames_skipped;

	uint8_t requested_date;

	TextLayer *date_layer;
	char date_layer_text[3];
} DateComplicationData;

static void date_complication_update(Layer *layer, GContext *ctx)
{
	base_complication_update(layer, ctx, GColorClear, 0);
}

DateComplication* date_complication_create(GRect frame)
{
	Layer *layer = layer_create_with_data(frame, sizeof(DateComplicationData));
	layer_set_update_proc(layer, date_complication_update);

	DateComplicationData *data = layer_get_data(layer);
	data->animating = true; // Must call date_complication_animate_in

	// As far as I can tell this is just a magic number.  3px margins per side?
	static const unsigned int text_margins = 6;
	static const unsigned int date_font_height = 20;
	static const unsigned int date_layer_height = text_margins + date_font_height;
	GRect date_frame = layer_get_bounds(layer);
	date_frame.origin.y = date_frame.size.h / 2 - date_layer_height / 2;
	date_frame.size.h = date_layer_height;
	data->date_layer_text[0] = '\0';
	data->date_layer = text_layer_create(date_frame);
	text_layer_set_background_color(data->date_layer, GColorClear);
	text_layer_set_text_color(data->date_layer, GColorDarkGray);
	text_layer_set_text_alignment(data->date_layer, GTextAlignmentCenter);
	text_layer_set_font(data->date_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
	text_layer_set_text(data->date_layer, (char*)&data->date_layer_text);
	layer_add_child(layer, text_layer_get_layer(data->date_layer));

	return (DateComplication*)layer;
}

void date_complication_destroy(DateComplication *complication)
{
	Layer *layer = date_complication_get_layer(complication);
	DateComplicationData *data = layer_get_data(layer);

	text_layer_destroy(data->date_layer);

	layer_destroy(layer);
}

inline Layer* date_complication_get_layer(DateComplication *complication)
{
	return (Layer*)complication;
}

static void date_complication_set_displayed(Layer *layer, DateComplicationData *data, uint8_t mday)
{
	snprintf(data->date_layer_text, NELEM(data->date_layer_text), "%02u", mday);
	layer_mark_dirty(layer);
}

void date_complication_time_changed(DateComplication *complication, struct tm *time)
{
	Layer *layer = date_complication_get_layer(complication);
	DateComplicationData *data = layer_get_data(layer);

	if(data->animating) {
		data->requested_date = time->tm_mday;
	}
	else {
		date_complication_set_displayed(layer, data, time->tm_mday);
	}
}

static void date_complication_spinup_animation_update(Animation* anim, AnimationProgress progress)
{
	DateComplication *complication = animation_get_context(anim);
	Layer *layer = date_complication_get_layer(complication);
	DateComplicationData *data = layer_get_data(layer);

	if(data->anim_frames_skipped == 1) {
		uint8_t random_date = rand() % 100;
		date_complication_set_displayed(layer, data, random_date);
		data->anim_frames_skipped = 0;
	}
	else {
		data->anim_frames_skipped++;
	}
}

static void date_complication_spinup_animation_started(Animation *animation, void *context)
{
	DateComplication *complication = context;
	Layer *layer = date_complication_get_layer(complication);
	DateComplicationData *data = layer_get_data(layer);

	data->animating = true;
	data->anim_frames_skipped = 0;
}

static void date_complication_spinup_animation_stopped(Animation *animation, bool finished, void *context)
{
	DateComplication *complication = context;
	Layer *layer = date_complication_get_layer(complication);
	DateComplicationData *data = layer_get_data(layer);

	data->animating = false;
	date_complication_set_displayed(layer, data, data->requested_date);
}

void date_complication_animate_in(DateComplication *complication)
{
	static const AnimationImplementation date_spinup_anim_impl = {
		.update = date_complication_spinup_animation_update
	};
	static const AnimationHandlers date_spinup_anim_handlers = {
		.started = date_complication_spinup_animation_started,
		.stopped = date_complication_spinup_animation_stopped
	};

	base_complication_animate_in(&date_spinup_anim_impl,
	                             &date_spinup_anim_handlers,
	                             complication);
}

