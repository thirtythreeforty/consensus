#include "face_layer.h"

#include "AnimationBuffer.h"
#include "common.h"
#include "themes.h"

static void draw_path_with(GContext *ctx, GPath* path, uint8_t stroke_width, GColor stroke_color)
{
	graphics_context_set_stroke_width(ctx, stroke_width);
	graphics_context_set_stroke_color(ctx, stroke_color);
	gpath_draw_outline(ctx, path);
}

template<int32_t Interval>
FaceLayer::Hand<Interval>::Hand(Boulder::Layer& layer, const GPathInfo *path_info, GPoint center)
	: angle(0, 1000)
	, scale(0, 1000)
	, path(path_info)
	, layer(layer)
{
	angle.set_callback(this);
	scale.set_callback(this);

	gpath_move_to(path, center);

	// initial state of scale does not trigger a callback
	path.scale(0);
}

template<int32_t Interval>
void FaceLayer::Hand<Interval>::zoom(bool in)
{
	if(in) {
		scale = ANIMATION_NORMALIZED_MAX;
	}
	else {
		scale = 0;
	}
}

template<int32_t Interval>
void FaceLayer::Hand<Interval>::on_animated_update(void *animated)
{
	if(animated == &angle) {
		gpath_rotate_to(path, angle);
	} else {
		path.scale(scale);
	}
	layer.mark_dirty();
}

FaceLayer::FaceLayer(GRect frame, bool large)
	: Layer(frame)
	, hour_hand(*this, large ? &hour_hand_path : &small_hour_hand_path, grect_center_point(&frame))
	, min_hand(*this, large ? &minute_hand_path : &small_minute_hand_path, grect_center_point(&frame))
	, sec_hand(*this, large ? &second_hand_path : &small_second_hand_path, grect_center_point(&frame))
	, large(large)
{
	hour_hand.zoom(true);
	min_hand.zoom(true);
}

void FaceLayer::set_show_second(bool show)
{
	sec_hand.zoom(show);
}

namespace {

static unsigned int hour_angle(unsigned int hour, unsigned int minute)
{
	return TRIG_MAX_ANGLE * (hour % 12) / 12 +
	       TRIG_MAX_ANGLE * minute / (12 * 60);
}

static unsigned int minute_angle(unsigned int minute, unsigned int second)
{
	return TRIG_MAX_ANGLE * minute / 60 +
	       TRIG_MAX_ANGLE * second / (60 * 60);
}

}

void FaceLayer::set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
	mark_dirty();

	hour_hand.set_angle(hour_angle(hour, min));
	min_hand.set_angle(minute_angle(min, sec));
	sec_hand.set_angle(minute_angle(sec, 0));
}

void FaceLayer::update(GContext *ctx)
{
	const uint8_t base_width = large ? theme().minute_hand_width() : 3;
	const uint8_t fatter_width = large ? base_width + 2 : base_width;
	const GColor hour_hand_color = large ? theme().hour_hand_color()
	                                     : theme().complication_icon_color();
	const GColor minute_hand_color = large ? theme().minute_hand_color()
	                                       : theme().complication_icon_color();
	const GColor second_hand_color = large ? theme().second_hand_color()
	                                       : theme().complication_icon_color();

#ifndef PBL_COLOR
	// Draw an extra outline to separate the hands from things behind them

	const uint8_t outline_extra = 2;
	if(large) {
		draw_path_with(ctx, hour_hand.get_path(), fatter_width + outline_extra, theme().background_color());
	}
#endif
	draw_path_with(ctx, hour_hand.get_path(), fatter_width, hour_hand_color);

#ifndef PBL_COLOR
	if(large) {
		draw_path_with(ctx, min_hand.get_path(), base_width + outline_extra, theme().background_color());
	}
#endif
	draw_path_with(ctx, min_hand.get_path(), base_width, minute_hand_color);

	if(large) {
		// Second hand
#ifndef PBL_COLOR
		draw_path_with(ctx, sec_hand.get_path(), base_width - 4 + outline_extra, theme().background_color());
#endif
		draw_path_with(ctx, sec_hand.get_path(), base_width - 4, second_hand_color);
	}

	// Draw middle dot, only if large though
	if(large) {
		GRect bounds = this->get_bounds();
		GPoint center = grect_center_point(&bounds);

		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_fill_color(ctx, theme().background_color());
		// Yes this is a little hacky, but it works fine
		graphics_fill_circle(ctx, center, base_width > 5 ? 2 : 1);
	}
}

const GPoint FaceLayer::hour_hand_path_points[] = {{0, 0}, {0, -35}};
const GPathInfo FaceLayer::hour_hand_path = {
	.num_points = NELEM(hour_hand_path_points),
	.points = (GPoint*)hour_hand_path_points
};
const GPoint FaceLayer::minute_hand_path_points[] = {{0, 0}, {0, -60}};
const GPathInfo FaceLayer::minute_hand_path = {
	.num_points = NELEM(minute_hand_path_points),
	.points = (GPoint*)minute_hand_path_points
};

// The extra {0, 0} point keeps the second hand always going through the
// center, even if the rotation makes the hand imperceptibly bent due to
// imperfect rotation.
const GPoint FaceLayer::second_hand_path_points[] = {{0, 15}, {0, 0}, {0, -60}};
const GPathInfo FaceLayer::second_hand_path = {
	.num_points = NELEM(second_hand_path_points),
	.points = (GPoint*)second_hand_path_points
};

const GPoint FaceLayer::small_hour_hand_path_points[] = {{0, 0}, {0, -10}};
const GPathInfo FaceLayer::small_hour_hand_path = {
	.num_points = NELEM(small_hour_hand_path_points),
	.points = (GPoint*)small_hour_hand_path_points
};
const GPoint FaceLayer::small_minute_hand_path_points[] = {{0, 0}, {0, -18}};
const GPathInfo FaceLayer::small_minute_hand_path = {
	.num_points = NELEM(small_minute_hand_path_points),
	.points = (GPoint*)small_minute_hand_path_points
};
const GPoint FaceLayer::small_second_hand_path_points[] = {{0, 0}, {0, -18}};
const GPathInfo FaceLayer::small_second_hand_path = {
	.num_points = NELEM(small_second_hand_path_points),
	.points = (GPoint*)small_second_hand_path_points
};
