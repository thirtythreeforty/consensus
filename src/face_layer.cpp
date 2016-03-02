#include "face_layer.h"

#include "common.h"
#include "themes.h"

static void draw_path_with(GContext *ctx, GPath* path, uint8_t stroke_width, GColor stroke_color)
{
	graphics_context_set_stroke_width(ctx, stroke_width);
	graphics_context_set_stroke_color(ctx, stroke_color);
	gpath_draw_outline(ctx, path);
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
		draw_path_with(ctx, hour_path, fatter_width + outline_extra, theme().background_color());
	}
#endif
	draw_path_with(ctx, hour_path, fatter_width, hour_hand_color);

#ifndef PBL_COLOR
	if(large) {
		draw_path_with(ctx, minute_path, base_width + outline_extra, theme().background_color());
	}
#endif
	draw_path_with(ctx, minute_path, base_width, minute_hand_color);

	if(show_second) {
#ifndef PBL_COLOR
		if(large) {
			draw_path_with(ctx, second_path, base_width - 4 + outline_extra, theme().background_color());
		}
#endif
		draw_path_with(ctx, second_path, base_width - 4, second_hand_color);
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

FaceLayer::FaceLayer(GRect frame, bool large)
	: Layer(frame)
	, animating(false)
	, hour_path(large ? &hour_hand_path : &small_hour_hand_path)
	, minute_path(large ? &minute_hand_path : &small_minute_hand_path)
	, second_path(large ? &second_hand_path : &small_second_hand_path)
	, show_second(false)
	, large(large)
{
	GPoint center = grect_center_point(&frame);

	gpath_move_to(hour_path, center);
	gpath_move_to(minute_path, center);
	gpath_move_to(second_path, center);
}

void FaceLayer::set_show_second(bool show)
{
	show_second = show;

	mark_dirty();
}

static unsigned int face_layer_hour_angle(unsigned int hour, unsigned int minute)
{
	return TRIG_MAX_ANGLE * (hour % 12) / 12 +
	       TRIG_MAX_ANGLE * minute / (12 * 60);
}

static unsigned int face_layer_minute_angle(unsigned int minute, unsigned int second)
{
	return TRIG_MAX_ANGLE * minute / 60 +
	       TRIG_MAX_ANGLE * second / (60 * 60);
}

void FaceLayer::set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
	requested_time.hour = hour;
	requested_time.minute = min;
	requested_time.second = sec;

	if(!animating) {
		gpath_rotate_to(hour_path, face_layer_hour_angle(hour, min));
		gpath_rotate_to(minute_path, face_layer_minute_angle(min, sec));
		gpath_rotate_to(second_path, face_layer_minute_angle(sec, 0));

		mark_dirty();
	}
}

/** Helper function to automate tedious setup in face_layer_animate_in */
Animation* FaceLayer::make_anim(int duration,
                                int delay,
                                AnimationCurve curve,
                                const AnimationImplementation *implementation,
                                const AnimationHandlers *handlers)
{
	static const AnimationHandlers dummy_handlers = {
		.started = NULL,
		.stopped = NULL,
	};
	Animation *anim = animation_create();
	animation_set_duration(anim, duration);
	animation_set_delay(anim, delay);
	animation_set_curve(anim, curve);
	animation_set_implementation(anim, implementation);
	animation_set_handlers(anim, handlers ? *handlers : dummy_handlers, this);

	return anim;
}

static int face_layer_scale(AnimationProgress dist_normalized, unsigned int max)
{
	return dist_normalized * max / ANIMATION_NORMALIZED_MAX;
}

void FaceLayer::radius_anim_update(Animation *anim, AnimationProgress dist_normalized)
{
	auto face_layer = static_cast<FaceLayer*>(animation_get_context(anim));

	face_layer->hour_path.scale(dist_normalized);
	face_layer->minute_path.scale(dist_normalized);
	face_layer->second_path.scale(dist_normalized);

	face_layer->mark_dirty();
}

void FaceLayer::radius_anim_setup(Animation *anim)
{
	auto face_layer = static_cast<FaceLayer*>(animation_get_context(anim));
	face_layer->animating = true;

	radius_anim_update(anim, 0);
}

void FaceLayer::roll_anim_setup(Animation *anim)
{
	// Set the initial roll so there's no flicker of large hands.
	auto face_layer = static_cast<FaceLayer*>(animation_get_context(anim));
	// We rely on the AnimationHandlers to set this to false when all the
	// animations are done.  But this needs to be true here to make sure the
	// hands are never displayed as the requested_time.
	face_layer->animating = true;
	roll_anim_update(anim, 0);
}

void FaceLayer::roll_anim_update(Animation *anim, AnimationProgress dist_normalized)
{
	auto face_layer = static_cast<FaceLayer*>(animation_get_context(anim));
	gpath_rotate_to(face_layer->hour_path,
	                face_layer_scale(dist_normalized,
	                                 face_layer_hour_angle(face_layer->animation_time.hour,
	                                                       face_layer->animation_time.minute)));

	gpath_rotate_to(face_layer->minute_path,
	                face_layer_scale(dist_normalized,
	                                 face_layer_minute_angle(face_layer->animation_time.minute,
	                                                         face_layer->animation_time.second)));

	gpath_rotate_to(face_layer->second_path,
	                face_layer_scale(dist_normalized,
	                                 face_layer_minute_angle(face_layer->animation_time.second, 0)));

	face_layer->mark_dirty();
}

// A couple more helper functions to create zoom and roll animations
Animation* FaceLayer::make_zoom_anim()
{
	static const AnimationImplementation zoom_anim_impl = {
		.setup = FaceLayer::radius_anim_setup,
		.update = FaceLayer::radius_anim_update,
		.teardown = NULL,
	};

	return make_anim(1000, 200,
	                 AnimationCurveEaseOut,
	                 &zoom_anim_impl,
	                 NULL);
}

Animation* FaceLayer::make_roll_anim()
{
	static const AnimationImplementation roll_anim_impl = {
		.setup = FaceLayer::roll_anim_setup,
		.update = FaceLayer::roll_anim_update,
		.teardown = NULL,
	};

	return make_anim(1000, 200,
	                 AnimationCurveEaseInOut,
	                 &roll_anim_impl,
	                 NULL);
}

void FaceLayer::animation_start_handler(Animation *animation, void *context)
{
	auto face_layer = static_cast<FaceLayer*>(context);
	face_layer->animating = true;

	// Grab a copy of the requested time to use during animation
	face_layer->animation_time = face_layer->requested_time;
}

void FaceLayer::animation_stop_handler(Animation *animation, bool finished, void *context)
{
	auto face_layer = static_cast<FaceLayer*>(context);
	face_layer->animating = false;

	// Set scale and roll to final values
	face_layer->hour_path.scale(ANIMATION_NORMALIZED_MAX);
	face_layer->minute_path.scale(ANIMATION_NORMALIZED_MAX);
	face_layer->second_path.scale(ANIMATION_NORMALIZED_MAX);

	// This will move the hands to requested_time and update the display
	face_layer->set_time(face_layer->requested_time.hour,
	                     face_layer->requested_time.minute,
	                     face_layer->requested_time.second);
}

Animation* FaceLayer::animate_in(bool zoom, bool roll)
{
	// Collect all animations into this array to create a spawn animation later
	Animation *animations[2];
	unsigned int i = 0;

	if(zoom) {
		animations[i++] = make_zoom_anim();
	}
	if(roll) {
		animations[i++] = make_roll_anim();
	}

	static const AnimationHandlers handlers = {
		.started = FaceLayer::animation_start_handler,
		.stopped = FaceLayer::animation_stop_handler
	};

	Animation *spawn_anim = animation_spawn_create_from_array(animations, i);
	animation_set_handlers(spawn_anim, handlers, this);

	return spawn_anim;
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
