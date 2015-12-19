#include "complication_common.h"

void base_complication_update(Layer *layer, GContext *ctx,
                              GColor color, int32_t max_angle)
{
	GRect bounds = layer_get_bounds(layer);

	// Pebble's graphics code does not draw entirely in the specified box, so
	// subtract a couple pixels to make sure the line doesn't clip the layer edge.
	const unsigned int line_size = 1;
	bounds.size.h -= (line_size + 2);
	bounds.size.w -= (line_size + 2);
	bounds.origin.x += (line_size / 2 + 1);
	bounds.origin.y += (line_size / 2 + 1);
	graphics_context_set_stroke_width(ctx, line_size);

	graphics_context_set_stroke_color(ctx, color);
	graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, 0, max_angle);

	graphics_context_set_stroke_color(ctx, GColorDarkGray);
	graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, max_angle, TRIG_MAX_ANGLE);
}
