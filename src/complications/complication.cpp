#include "complication.h"

#include "common.h"

const unsigned int complication_line_size = 2;
const unsigned int complication_background_line_size = 1;

static void base_complication_appropriate_bounds(GRect& bounds)
{
	// Pebble's graphics code does not draw entirely in the specified box, so
	// subtract a couple pixels to make sure the line doesn't clip the layer edge.
	bounds.size.h -= (complication_line_size + 3);
	bounds.size.w -= (complication_line_size + 3);
	bounds.origin.x += (complication_line_size / 2 + 1);
	bounds.origin.y += (complication_line_size / 2 + 1);
}

static void draw_arc(GContext *ctx, GRect& bounds, GColor color,
                     int32_t min_angle, int32_t angle, int32_t max_angle)
{
	angle = clamp((long int)0, angle, max_angle - min_angle);
	if(angle > 0) {
		graphics_context_set_stroke_width(ctx, complication_line_size);
		graphics_context_set_stroke_color(ctx, color);
		graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, min_angle, min_angle + angle);
	}

	if(angle < max_angle) {
		graphics_context_set_stroke_width(ctx, complication_background_line_size);
		graphics_context_set_stroke_color(ctx, GColorDarkGray);
		graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, min_angle + angle, max_angle);
	}
}

void Complication::update(GContext* ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, GColorClear, 0, 0, TRIG_MAX_ANGLE);
}

void HighlightComplication::update(GContext* ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, highlight_color(), 0, angle, TRIG_MAX_ANGLE);
}

void HighlightComplication2::update(GContext *ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, highlight_color(), 0, angle, TRIG_MAX_ANGLE / 2);
	draw_arc(ctx, bounds, highlight_color2(), TRIG_MAX_ANGLE / 2, angle2, TRIG_MAX_ANGLE);
}

void HighlightComplication::set_angle(angle_t new_angle)
{
	if(requested_angle == new_angle) {
		return;
	}

	requested_angle = new_angle;

	if(!animating) {
		animate_to_requested();
	}
}

void HighlightComplication2::set_angle2(angle_t new_angle)
{
	if(requested_angle2 == new_angle) {
		return;
	}

	requested_angle2 = new_angle;

	if(!animating2) {
		animate_to_requested2();
	}
}

void HighlightComplication::animate_to_requested()
{
	animating = true;
	// TODO if it's a small change, don't bother animating

	using Boulder::PropertyAnimationGetter;
	using Boulder::PropertyAnimationSetter;

	Boulder::PropertyAnimation<
		HighlightComplication, angle_t,
		panim_set_angle, panim_get_angle
	> anim(*this, &angle, &requested_angle);

	static const AnimationHandlers handlers = {
		.started = NULL,
		.stopped = HighlightComplication::panim_stopped,
	};
	base_setup_animation(anim, handlers);
	anim.schedule();
}

void HighlightComplication2::animate_to_requested2()
{
	animating2 = true;
	// TODO if it's a small change, don't bother animating

	using Boulder::PropertyAnimationGetter;
	using Boulder::PropertyAnimationSetter;

	Boulder::PropertyAnimation<
		HighlightComplication2, angle_t,
		panim_set_angle2, panim_get_angle2
	> anim(*this, &this->angle2, &requested_angle2);

	static const AnimationHandlers handlers = {
		.started = NULL,
		.stopped = HighlightComplication2::panim_stopped2,
	};
	base_setup_animation(anim, handlers);
	anim.schedule();
}

void HighlightComplication::panim_stopped(Animation *anim, bool finished, void *context)
{
	auto *complication = static_cast<HighlightComplication*>(context);

	complication->animating = false;

	if(complication->requested_angle != complication->angle) {
		complication->animate_to_requested();
	}
}

void HighlightComplication2::panim_stopped2(Animation *anim, bool finished, void *context)
{
	auto *complication = static_cast<HighlightComplication2*>(context);

	complication->animating2 = false;

	if(complication->requested_angle2 != complication->angle2) {
		complication->animate_to_requested2();
	}
}

void HighlightComplication::panim_set_angle(HighlightComplication& complication, const angle_t& new_angle) {
	complication.angle = new_angle;
	complication.mark_dirty();
}

auto HighlightComplication::panim_get_angle(const HighlightComplication& complication) -> const angle_t& {
	return complication.angle;
}

void HighlightComplication2::panim_set_angle2(HighlightComplication2& complication, const angle_t& new_angle) {
	complication.angle2 = new_angle;
	complication.mark_dirty();
}

auto HighlightComplication2::panim_get_angle2(const HighlightComplication2& complication) -> const angle_t& {
	return complication.angle2;
}

void Complication::base_setup_animation(Animation *anim, const AnimationHandlers &handlers)
{
	static const unsigned int duration = 700;
	static const unsigned int delay = 200;

	animation_set_duration(anim, duration);
	animation_set_delay(anim, delay);
	animation_set_curve(anim, AnimationCurveEaseInOut);
	animation_set_handlers(anim, handlers, this);
}
