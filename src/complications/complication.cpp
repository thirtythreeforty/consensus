#include "complication.h"

#include "common.h"
#include "themes.h"

const unsigned int complication_line_size = 2;
const unsigned int complication_background_line_size = 1;

static void base_complication_appropriate_bounds(GRect& bounds)
{
	// Pebble's graphics code does not draw entirely in the specified box, so
	// subtract a couple pixels to make sure the line doesn't clip the layer edge.
	bounds.size.h -= 5;
	bounds.size.w -= 5;
	bounds.origin.x += 2;
	bounds.origin.y += 2;
}

static void draw_arc(GContext *ctx, GRect& bounds, GColor color,
                     int32_t min_angle, int32_t angle, int32_t max_angle)
{
	angle = clamp((long int)0, angle, max_angle - min_angle);

	if(angle < max_angle) {
		graphics_context_set_stroke_width(ctx, complication_background_line_size);
		if(theme().complication_fill) {
			graphics_context_set_fill_color(ctx, theme().complication_ring_background_color);
			graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10000, min_angle, max_angle);
		}
		else {
			graphics_context_set_stroke_color(ctx, theme().complication_ring_background_color);
			graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, min_angle + angle, max_angle);
		}
	}

	if(angle > 0) {
		graphics_context_set_stroke_width(ctx, theme().complication_ring_thickness);
		graphics_context_set_stroke_color(ctx, color);
		graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, min_angle, min_angle + angle);
	}
}

void Complication::configure(const config_bundle_t& config)
{}

void Complication::update(GContext* ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, GColorClear, 0, 0, TRIG_MAX_ANGLE);
}

IconTextComplication::IconTextComplication(GRect frame)
	: Complication(frame)
	, number(get_bounds(), "")
	, icon()
{
	add_child(number.get_text_layer());
}

void IconTextComplication::configure(const std::array<unsigned int, 4>& config)
{
	Complication::configure(config);

	icon.recolor();
	number.reconfigure_color();
	mark_dirty();
}

void IconTextComplication::update(GContext *ctx)
{
	// This may not be the best architecture, because this is currently called
	// as a "helper" function from HighlightComplication's update function.
	// But (at some point) we will have IconTextComplications that are not
	// HighlightComplication, like the proposed compass complication.

	icon.draw(ctx);
	// text will draw itself; it's a sublayer
}

void IconTextComplication::set_icon(uint32_t resource_id) {
	icon.reset(resource_id, get_bounds());
	mark_dirty();
}

void IconTextComplication::reset_icon() {
	icon.reset();
	mark_dirty();
}

void IconTextComplication::set_number(int32_t n) {
	number.set(n);
}

void IconTextComplication::set_number_format(const char* fmt, int32_t n) {
	number.set_format(fmt, n);
}

void HighlightComplication::update(GContext* ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, highlight_color(), 0, angle, TRIG_MAX_ANGLE);

	IconTextComplication::update(ctx);
}

void HighlightComplication2::update(GContext *ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, highlight_color(), 0, angle, TRIG_MAX_ANGLE / 2);
	draw_arc(ctx, bounds, highlight_color2(), TRIG_MAX_ANGLE / 2, angle2, TRIG_MAX_ANGLE);

	IconTextComplication::update(ctx);
}

void HighlightComplication::set_angle(angle_t new_angle)
{
	if(requested_angle == new_angle) {
		return;
	}

	// Clamp the requested angle to make sure the animations are at the correct speed
	requested_angle = std::min(std::max(new_angle, angle_t(0)), angle_t(TRIG_MAX_ANGLE));

	if(!animating) {
		animate_to_requested();
	}
}

void HighlightComplication2::set_angle2(angle_t new_angle)
{
	if(requested_angle2 == new_angle) {
		return;
	}

	// Clamp the requested angle to make sure the animations are at the correct speed
	requested_angle2 = std::min(std::max(new_angle, angle_t(0)), angle_t(TRIG_MAX_ANGLE));

	if(!animating2) {
		animate_to_requested2();
	}
}

void HighlightComplication::animate_to_requested()
{
	// if it's a small change, don't bother animating
	if(abs(angle - requested_angle) < TRIG_MAX_ANGLE / 100) {
		angle = requested_angle;
		mark_dirty();
	}
	else {
		animating = true;

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
}

void HighlightComplication2::animate_to_requested2()
{
	// if it's a small change, don't bother animating
	if(abs(angle2 - requested_angle2) < TRIG_MAX_ANGLE / 100) {
		angle2 = requested_angle2;
		mark_dirty();
	}
	else {
		animating2 = true;

		using Boulder::PropertyAnimationGetter;
		using Boulder::PropertyAnimationSetter;

		Boulder::PropertyAnimation<
			HighlightComplication2, angle_t,
			panim_set_angle2, panim_get_angle2
		> anim(*this, &angle2, &requested_angle2);

		static const AnimationHandlers handlers = {
			.started = NULL,
			.stopped = HighlightComplication2::panim_stopped2,
		};
		base_setup_animation(anim, handlers);
		anim.schedule();
	}
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
