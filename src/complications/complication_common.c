#include "complication_common.h"

const unsigned int complication_line_size = 2;
const unsigned int complication_background_line_size = 1;

static void base_complication_appropriate_bounds(GRect *bounds)
{
	// Pebble's graphics code does not draw entirely in the specified box, so
	// subtract a couple pixels to make sure the line doesn't clip the layer edge.
	bounds->size.h -= (complication_line_size + 3);
	bounds->size.w -= (complication_line_size + 3);
	bounds->origin.x += (complication_line_size / 2 + 1);
	bounds->origin.y += (complication_line_size / 2 + 1);
}

static void base_complication_base_update(GContext *ctx, GRect *bounds, GColor color,
                                          int32_t min_angle, int32_t angle, int32_t max_angle)
{
	if(angle > 0) {
		graphics_context_set_stroke_width(ctx, complication_line_size);
		graphics_context_set_stroke_color(ctx, color);
		graphics_draw_arc(ctx, *bounds, GOvalScaleModeFitCircle, min_angle, min_angle + angle);
	}

	if(angle < max_angle) {
		graphics_context_set_stroke_width(ctx, complication_background_line_size);
		graphics_context_set_stroke_color(ctx, GColorDarkGray);
		graphics_draw_arc(ctx, *bounds, GOvalScaleModeFitCircle, min_angle + angle, max_angle);
	}
}

void base_complication_update_1(Layer *layer, GContext *ctx,
                                GColor color, int32_t max_angle)
{
	GRect bounds = layer_get_bounds(layer);

	base_complication_appropriate_bounds(&bounds);

	base_complication_base_update(ctx, &bounds, color, 0, max_angle, TRIG_MAX_ANGLE);
}

void base_complication_update_2(Layer *layer, GContext *ctx,
                                GColor color_1, int32_t max_angle_1,
                                GColor color_2, int32_t max_angle_2)
{
	GRect bounds = layer_get_bounds(layer);

	base_complication_appropriate_bounds(&bounds);

	base_complication_base_update(ctx, &bounds, color_1, 0, max_angle_1, TRIG_MAX_ANGLE / 2);
	base_complication_base_update(ctx, &bounds, color_2, TRIG_MAX_ANGLE / 2, max_angle_2, TRIG_MAX_ANGLE);
}

Animation* base_complication_animate_in(const AnimationImplementation *impl,
                                        const AnimationHandlers *handlers, void *ctx)
{
	static const unsigned int duration = 700;
	static const unsigned int delay = 200;

	Animation *anim = animation_create();
	animation_set_duration(anim, duration);
	animation_set_delay(anim, delay);
	animation_set_curve(anim, AnimationCurveEaseInOut);
	animation_set_implementation(anim, impl);
	animation_set_handlers(anim, *handlers, ctx);

	return anim;
}
