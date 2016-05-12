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

static void draw_foreground_arc(GContext *ctx, GRect& bounds,
                                uint8_t stroke_width, GColor stroke_color,
                                int32_t min_angle, int32_t max_angle)
{
		graphics_context_set_stroke_width(ctx, stroke_width);
		graphics_context_set_stroke_color(ctx, stroke_color);
		graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, min_angle, max_angle);
}

static void draw_arc(GContext *ctx, GRect& bounds, GColor color,
                     int32_t min_angle, int32_t angle, int32_t max_angle)
{
	angle = clamp((long int)0, angle, max_angle - min_angle);

	if(angle < max_angle) {
		graphics_context_set_stroke_width(ctx, complication_background_line_size);
		if(theme().complication_fill()) {
			graphics_context_set_fill_color(ctx, theme().complication_ring_background_color());
			graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 10000, min_angle, max_angle);
		}
		else {
			graphics_context_set_stroke_color(ctx, theme().complication_ring_background_color());
			graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, min_angle + angle, max_angle);
		}
	}

	if(angle > 0) {
		draw_foreground_arc(ctx, bounds, theme().complication_ring_thickness(),
		                    color, min_angle, min_angle + angle);
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
	, number(get_bounds(), empty)
	, unit_text(calculate_unit_frame(frame))
	, icon()
{
	add_child(number.get_text_layer());
	add_child(unit_text);
	unit_text.set_text_alignment(GTextAlignmentCenter);
	unit_text.set_background_color(GColorClear);
	unit_text.set_font(fonts_get_system_font(FONT_KEY_GOTHIC_09));
}

GRect IconTextComplication::calculate_unit_frame(const GRect& c_frame)
{
	static constexpr int16_t unit_height = 12;
	return {
		//HACK
		.origin = GPoint(0, int16_t(c_frame.size.h / 2 - unit_height / 2 + 13)),
		.size = GSize(c_frame.size.w, unit_height)
	};
}

void IconTextComplication::configure(const std::array<unsigned int, 4>& config)
{
	Complication::configure(config);

	icon.recolor();
	unit_text.set_text_color(theme().complication_text_color());
	number.reconfigure_color();
	mark_dirty();
}

void IconTextComplication::update(GContext *ctx)
{
	Complication::update(ctx);

	icontext_update(ctx);
}

void IconTextComplication::icontext_update(GContext *ctx)
{
	icon.draw(ctx);
	// text will draw itself; it's a sublayer
}

void IconTextComplication::set_icon(uint32_t resource_id) {
	icon.reset(resource_id, get_bounds());
	mark_dirty();
	reset_number();
}

void IconTextComplication::reset_icon() {
	icon.reset();
	mark_dirty();
}

void IconTextComplication::set_number(int32_t n) {
	number.set(n);
	reset_icon();
}

void IconTextComplication::set_number_format(const char* fmt, int32_t n, const char* unit) {
	number.set_format(fmt, n);
	unit_text.set_text(unit);
	reset_icon();
}

void IconTextComplication::reset_number()
{
	number.set_format(empty, 0);
	unit_text.set_text(empty);
}

HighlightComplication::HighlightComplication(GRect frame)
	: IconTextComplication(frame)
{
	angle.set_callback(this);
}

HighlightComplication2::HighlightComplication2(GRect frame)
	: HighlightComplication(frame)
{
	angle2.set_callback(this);
}

bool HighlightComplication::angles_are_close(const angle_t& a, const angle_t& b)
{
	return abs(a - b) < TRIG_MAX_ANGLE / 100;
}

auto HighlightComplication::clamp_angle(const angle_t& angle) -> angle_t
{
	return std::min(std::max(angle, (angle_t)0), (angle_t)TRIG_MAX_ANGLE);
}


void HighlightComplication::update(GContext* ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, highlight_color(), 0, angle, TRIG_MAX_ANGLE);

	icontext_update(ctx);
}

TickComplication::TickComplication(GRect frame)
	: HighlightComplication(frame)
{
	tick_angle.set_callback(this);
}

void TickComplication::update(GContext* ctx)
{
	HighlightComplication::update(ctx);

	// Draw the yellow "average steps now" tick mark
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	constexpr unsigned int tick_thickness = TRIG_MAX_ANGLE / 1800;
	draw_foreground_arc(ctx, bounds, theme().complication_ring_thickness() + 2,
	                    tick_color(), tick_angle - tick_thickness / 2, tick_angle + tick_thickness / 2);
}

void HighlightComplication2::update(GContext *ctx)
{
	GRect bounds = this->get_bounds();
	base_complication_appropriate_bounds(bounds);

	draw_arc(ctx, bounds, highlight_color(), 0, angle, TRIG_MAX_ANGLE / 2);
	draw_arc(ctx, bounds, highlight_color2(), TRIG_MAX_ANGLE / 2, angle2, TRIG_MAX_ANGLE);

	icontext_update(ctx);
}

void HighlightComplication::on_animated_update()
{
	mark_dirty();
}
