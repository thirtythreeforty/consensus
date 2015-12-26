#include "face_layer.h"

#include "common.h"
#include "scalable_path.h"

typedef struct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} hms_t;

struct st_face_layer_data {
	hms_t requested_time;
	hms_t animation_time;

	bool animating;

	ScalablePath *hour_path;
	ScalablePath *minute_path;
	ScalablePath *second_path;

	bool show_second;

	GColor hour_color;
	GColor minute_color;
	GColor second_color;
};
typedef struct st_face_layer_data FaceLayerData;

static void face_layer_redraw(Layer *layer, GContext *ctx)
{
	FaceLayerData *face_layer_data = layer_get_data(layer);

	graphics_context_set_stroke_width(ctx, 7);
	graphics_context_set_stroke_color(ctx, face_layer_data->hour_color);
	gpath_draw_outline(ctx, scalable_path_get_path(face_layer_data->hour_path));

	graphics_context_set_stroke_width(ctx, 5);
	graphics_context_set_stroke_color(ctx, face_layer_data->minute_color);
	gpath_draw_outline(ctx, scalable_path_get_path(face_layer_data->minute_path));

	if(face_layer_data->show_second) {
		graphics_context_set_stroke_width(ctx, 1);
		graphics_context_set_stroke_color(ctx, face_layer_data->second_color);
		gpath_draw_outline_open(ctx, scalable_path_get_path(face_layer_data->second_path));
	}

	// Middle dot
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, center, 1);
}

FaceLayer *face_layer_create(GRect bounds)
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

	FaceLayer *layer = layer_create_with_data(bounds, sizeof(FaceLayerData));
	layer_set_update_proc(layer, face_layer_redraw);

	FaceLayerData *face_layer_data = layer_get_data(layer);

	face_layer_data->animating = true; // Must call face_layer_animate_in at some point

	GPoint center = grect_center_point(&bounds);

	face_layer_data->hour_path = scalable_path_create(&hour_hand_path);
	gpath_move_to(scalable_path_get_path(face_layer_data->hour_path), center);

	face_layer_data->minute_path = scalable_path_create(&minute_hand_path);
	gpath_move_to(scalable_path_get_path(face_layer_data->minute_path), center);

	face_layer_data->second_path = scalable_path_create(&second_hand_path);
	gpath_move_to(scalable_path_get_path(face_layer_data->second_path), center);

	return layer;
}

void face_layer_destroy(FaceLayer *face_layer)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);
	scalable_path_destroy(face_layer_data->hour_path);
	scalable_path_destroy(face_layer_data->minute_path);
	scalable_path_destroy(face_layer_data->second_path);

	layer_destroy(face_layer);
}

void face_layer_set_show_second(FaceLayer *face_layer, bool show)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->show_second = show;

	layer_mark_dirty(face_layer);
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

void face_layer_set_time(FaceLayer *face_layer, uint8_t hour, uint8_t min, uint8_t sec)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->requested_time.hour = hour;
	face_layer_data->requested_time.minute = min;
	face_layer_data->requested_time.second = sec;

	if(!face_layer_data->animating) {
		gpath_rotate_to(scalable_path_get_path(face_layer_data->hour_path), face_layer_hour_angle(hour, min));
		gpath_rotate_to(scalable_path_get_path(face_layer_data->minute_path), face_layer_minute_angle(min, sec));
		gpath_rotate_to(scalable_path_get_path(face_layer_data->second_path), face_layer_minute_angle(sec, 0));

		layer_mark_dirty(face_layer);
	}
}

void face_layer_set_colors(FaceLayer *face_layer, GColor hour, GColor minute, GColor second)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->hour_color = hour;
	face_layer_data->minute_color = minute;
	face_layer_data->second_color = second;
}

/** Helper function to automate tedious setup in face_layer_animate_in */
static void face_layer_animate(int duration,
                               int delay,
                               AnimationCurve curve,
                               const AnimationImplementation *implementation,
                               const AnimationHandlers *handlers,
                               void *context)
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
	animation_set_handlers(anim, handlers ? *handlers : dummy_handlers, context);
	animation_schedule(anim);
}

static int face_layer_scale(AnimationProgress dist_normalized, unsigned int max)
{
	return dist_normalized * max / ANIMATION_NORMALIZED_MAX;
}

static void face_layer_radius_anim_update(Animation *anim, AnimationProgress dist_normalized)
{
	FaceLayer *face_layer = animation_get_context(anim);
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	scalable_path_scale(face_layer_data->hour_path, dist_normalized);
	scalable_path_scale(face_layer_data->minute_path, dist_normalized);
	scalable_path_scale(face_layer_data->second_path, dist_normalized);

	layer_mark_dirty(face_layer);
}

static void face_layer_roll_anim_update(Animation *anim, AnimationProgress dist_normalized)
{
	FaceLayer *face_layer = animation_get_context(anim);
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	gpath_rotate_to(scalable_path_get_path(face_layer_data->hour_path),
	                face_layer_scale(dist_normalized,
	                                 face_layer_hour_angle(face_layer_data->animation_time.hour,
	                                                       face_layer_data->animation_time.minute)));

	gpath_rotate_to(scalable_path_get_path(face_layer_data->minute_path),
	                face_layer_scale(dist_normalized,
	                                 face_layer_minute_angle(face_layer_data->animation_time.minute,
	                                                         face_layer_data->animation_time.second)));

	gpath_rotate_to(scalable_path_get_path(face_layer_data->second_path),
	                face_layer_scale(dist_normalized,
	                                 face_layer_minute_angle(face_layer_data->animation_time.second, 0)));

	layer_mark_dirty(face_layer);
}

static void face_layer_animation_start_handler(Animation *animation, void *context)
{
	FaceLayer *face_layer = context;
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->animating = true;

	// Grab a copy of the requested time to use during animation
	face_layer_data->animation_time = face_layer_data->requested_time;
}

static void face_layer_animation_stop_handler(Animation *animation, bool finished, void *context)
{
	FaceLayer *face_layer = context;
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->animating = false;

	// This will set the hands to requested_time and update the display
	face_layer_set_time(face_layer,
	                    face_layer_data->requested_time.hour,
	                    face_layer_data->requested_time.minute,
	                    face_layer_data->requested_time.second);
}

void face_layer_animate_in(FaceLayer *face_layer, bool zoom, bool roll)
{
	static const unsigned int duration = 1000;
	static const unsigned int delay = 0;
	static const AnimationImplementation radius_anim_impl = {
		.update = face_layer_radius_anim_update
	};
	static const AnimationImplementation roll_anim_impl = {
		.update = face_layer_roll_anim_update
	};

	// The "end" handler should be scheduled at the end of the longer running animation, because it signals
	// when it is OK to move the hands normally in face_layer_set_time
	static const AnimationHandlers handlers = {
		.started = face_layer_animation_start_handler,
		.stopped = face_layer_animation_stop_handler
	};

	if(zoom) {
		face_layer_animate(duration, delay, AnimationCurveEaseOut, &radius_anim_impl, roll ? NULL : &handlers, face_layer);

		// Go ahead and set the initial scale so that there's no flicker of large hands
		FaceLayerData *face_layer_data = layer_get_data(face_layer);
		scalable_path_scale(face_layer_data->hour_path, 0);
		scalable_path_scale(face_layer_data->minute_path, 0);
		scalable_path_scale(face_layer_data->second_path, 0);
	}
	if(roll) {
		face_layer_animate(duration, delay, AnimationCurveEaseInOut, &roll_anim_impl, &handlers, face_layer);
	}
	if(!zoom && !roll) {
		FaceLayerData *face_layer_data = layer_get_data(face_layer);
		face_layer_data->animating = false;
	}
}
