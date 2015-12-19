#include "complication_common.h"

#include "common.h"

typedef struct {
	uint8_t mday;
	uint8_t wday;
} day_date;

typedef struct {
	bool animating;
	day_date requested_date;
	day_date displayed_date;

	TextLayer *date_layer;
	char date_layer_text[3];
} DateComplicationData;

static void date_complication_update(Layer *layer, GContext *ctx)
{
	DateComplicationData *data = layer_get_data(layer);

	base_complication_update(layer, ctx, GColorClear, 0);

	snprintf(data->date_layer_text, NELEM(data->date_layer_text), "%u", data->displayed_date.mday);
}

#include "common.h"

DateComplication* date_complication_create(GRect bounds)
{
	Layer *layer = layer_create_with_data(bounds, sizeof(DateComplicationData));
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
	Layer *layer = (Layer*)complication;
	DateComplicationData *data = layer_get_data(layer);

	text_layer_destroy(data->date_layer);

	layer_destroy(layer);
}

Layer* date_complication_get_layer(DateComplication *complication)
{
	return (Layer*)complication;
}

void date_complication_time_changed(DateComplication *complication, struct tm *time)
{
	Layer *layer = (Layer*)complication;
	DateComplicationData *data = layer_get_data(layer);

	if(data->animating) {
		data->requested_date.wday = time->tm_wday;
		data->requested_date.mday = time->tm_mday;
	}
	else {
		data->displayed_date.wday = time->tm_wday;
		data->displayed_date.mday = time->tm_mday;
	}

	layer_mark_dirty(layer);
}

void date_complication_animate_in(DateComplication *complication)
{
	Layer *layer = date_complication_get_layer(complication);
	DateComplicationData *data = layer_get_data(layer);

	// TODO
	data->animating = false;
}

