#include "face_layer.h"

#include "common.h"

void FaceLayer::update(GContext *ctx)
{
	graphics_context_set_stroke_width(ctx, 7);
	graphics_context_set_stroke_color(ctx, hour_color);
	gpath_draw_outline(ctx, scalable_path_get_path(hour_path));

	graphics_context_set_stroke_width(ctx, 5);
	graphics_context_set_stroke_color(ctx, minute_color);
	gpath_draw_outline(ctx, scalable_path_get_path(minute_path));

	if(show_second) {
		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_stroke_color(ctx, second_color);
		gpath_draw_outline_open(ctx, scalable_path_get_path(second_path));
	}

	// Middle dot
	GRect bounds = this->get_bounds();
	GPoint center = grect_center_point(&bounds);

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, 1);
}

FaceLayer::FaceLayer(GRect frame)
	: Layer(frame)
	, animating(false)
	, show_second(false)
{
	static const GPoint hour_hand_path_points[] = {{0, 0}, {0, -35}};
	static const GPathInfo hour_hand_path = {
		.num_points = NELEM(hour_hand_path_points),
		.points = (GPoint*)hour_hand_path_points
	};
	static const GPoint minute_hand_path_points[] = {{0, 0}, {0, -55}};
	static const GPathInfo minute_hand_path = {
		.num_points = NELEM(minute_hand_path_points),
		.points = (GPoint*)minute_hand_path_points
	};
	// The extra {0, 0} point keeps the second hand always going through the center,
	// even if the rotation makes the hand imperceptibly bent due to imperfect rotation.
	static const GPoint second_hand_path_points[] = {{0, 15}, {0, 0}, {0, -57}};
	static const GPathInfo second_hand_path = {
		.num_points = NELEM(second_hand_path_points),
		.points = (GPoint*)second_hand_path_points
	};

	GPoint center = grect_center_point(&frame);

	// Create the hands
	hour_path = scalable_path_create(&hour_hand_path);
	gpath_move_to(scalable_path_get_path(hour_path), center);

	minute_path = scalable_path_create(&minute_hand_path);
	gpath_move_to(scalable_path_get_path(minute_path), center);

	second_path = scalable_path_create(&second_hand_path);
	gpath_move_to(scalable_path_get_path(second_path), center);
}

FaceLayer::~FaceLayer()
{
	scalable_path_destroy(hour_path);
	scalable_path_destroy(minute_path);
	scalable_path_destroy(second_path);
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
		gpath_rotate_to(scalable_path_get_path(hour_path), face_layer_hour_angle(hour, min));
		gpath_rotate_to(scalable_path_get_path(minute_path), face_layer_minute_angle(min, sec));
		gpath_rotate_to(scalable_path_get_path(second_path), face_layer_minute_angle(sec, 0));

		mark_dirty();
	}
}

void FaceLayer::set_colors(GColor hour, GColor minute, GColor second)
{
	hour_color = hour;
	minute_color = minute;
	second_color = second;

	mark_dirty();
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
	scalable_path_scale(face_layer->hour_path, dist_normalized);
	scalable_path_scale(face_layer->minute_path, dist_normalized);
	scalable_path_scale(face_layer->second_path, dist_normalized);

	face_layer->mark_dirty();
}

void FaceLayer::radius_anim_setup(Animation *anim)
{
	// Set the initial scale so there's no flicker of large hands.
	radius_anim_update(anim, 0);
}

void FaceLayer::roll_anim_update(Animation *anim, AnimationProgress dist_normalized)
{
	auto face_layer = static_cast<FaceLayer*>(animation_get_context(anim));
	gpath_rotate_to(scalable_path_get_path(face_layer->hour_path),
	                face_layer_scale(dist_normalized,
	                                 face_layer_hour_angle(face_layer->animation_time.hour,
	                                                       face_layer->animation_time.minute)));

	gpath_rotate_to(scalable_path_get_path(face_layer->minute_path),
	                face_layer_scale(dist_normalized,
	                                 face_layer_minute_angle(face_layer->animation_time.minute,
	                                                         face_layer->animation_time.second)));

	gpath_rotate_to(scalable_path_get_path(face_layer->second_path),
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
		.setup = NULL,
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
	scalable_path_scale(face_layer->hour_path, ANIMATION_NORMALIZED_MAX);
	scalable_path_scale(face_layer->minute_path, ANIMATION_NORMALIZED_MAX);
	scalable_path_scale(face_layer->second_path, ANIMATION_NORMALIZED_MAX);

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
