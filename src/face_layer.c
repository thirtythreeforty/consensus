#include "face_layer.h"

#include "common.h"

struct st_face_layer_data {
	struct {
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	} requested_time;

	bool animating;

	GPath *hour_path;
	GPath *minute_path;

	GColor hour_color;
	GColor minute_color;
};
typedef struct st_face_layer_data FaceLayerData;

static void face_layer_redraw(Layer *layer, GContext *ctx)
{
	FaceLayerData *face_layer_data = layer_get_data(layer);

	graphics_context_set_stroke_width(ctx, 7);
	graphics_context_set_stroke_color(ctx, face_layer_data->hour_color);
	gpath_draw_outline(ctx, face_layer_data->hour_path);

	graphics_context_set_stroke_width(ctx, 5);
	graphics_context_set_stroke_color(ctx, face_layer_data->minute_color);
	gpath_draw_outline(ctx, face_layer_data->minute_path);

	// Middle dot
	GRect bounds = layer_get_bounds(layer);
	GPoint center = grect_center_point(&bounds);

	graphics_context_set_stroke_width(ctx, 1);
	graphics_context_set_stroke_color(ctx, GColorBlack);
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

	FaceLayer *layer = layer_create_with_data(bounds, sizeof(FaceLayerData));
	layer_set_update_proc(layer, face_layer_redraw);

	FaceLayerData *face_layer_data = layer_get_data(layer);

	face_layer_data->animating = true; // Must call face_layer_animate_in at some point

	GPoint center = grect_center_point(&bounds);

	face_layer_data->hour_path = gpath_create(&hour_hand_path);
	gpath_move_to(face_layer_data->hour_path, center);

	face_layer_data->minute_path = gpath_create(&minute_hand_path);
	gpath_move_to(face_layer_data->minute_path, center);

	return layer;
}

void face_layer_destroy(FaceLayer *face_layer)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);
	gpath_destroy(face_layer_data->hour_path);
	gpath_destroy(face_layer_data->minute_path);

	layer_destroy(face_layer);
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

	APP_DEBUG("Requested time set to %i:%i:%i", hour, min, sec);

	if(!face_layer_data->animating) {
		gpath_rotate_to(face_layer_data->hour_path, face_layer_hour_angle(hour, min));
		gpath_rotate_to(face_layer_data->minute_path, face_layer_minute_angle(min, sec));
	}

	layer_mark_dirty(face_layer);
}

void face_layer_set_colors(FaceLayer *face_layer, GColor hour, GColor minute)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->hour_color = hour;
	face_layer_data->minute_color = minute;
}

/** Helper function to automate tedious setup in face_layer_animate_in */
static void face_layer_animate(int duration,
                               int delay,
                               AnimationCurve curve,
                               const AnimationImplementation *implementation,
                               const AnimationHandlers *handlers,
                               void *context)
{
	Animation *anim = animation_create();
	animation_set_duration(anim, duration);
	animation_set_delay(anim, delay);
	animation_set_curve(anim, curve);
	animation_set_implementation(anim, implementation);
	if(handlers != NULL) {
		animation_set_handlers(anim, *handlers, context);
	}
	animation_schedule(anim);
}

static int face_layer_scale(AnimationProgress dist_normalized, unsigned int max)
{
	return dist_normalized * max / ANIMATION_NORMALIZED_MAX;
}

static void face_layer_radius_anim_update(Animation *anim, AnimationProgress dist_normalized)
{

}

static void face_layer_roll_anim_update(Animation *anim, AnimationProgress dist_normalized)
{
	FaceLayer *face_layer = animation_get_context(anim);
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	APP_DEBUG("Roll animation progress: %04lx", dist_normalized);

	gpath_rotate_to(face_layer_data->hour_path,
	                face_layer_scale(dist_normalized,
	                                 face_layer_hour_angle(face_layer_data->requested_time.hour,
	                                                       face_layer_data->requested_time.minute)));

	gpath_rotate_to(face_layer_data->minute_path,
	                face_layer_scale(dist_normalized,
	                                 face_layer_minute_angle(face_layer_data->requested_time.minute,
	                                                       face_layer_data->requested_time.second)));

	layer_mark_dirty(face_layer);
}

static void face_layer_animation_start_handler(Animation *animation, void *context)
{
	FaceLayer *face_layer = context;
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->animating = true;

	APP_DEBUG("Animation started");
}

static void face_layer_animation_stop_handler(Animation *animation, bool finished, void *context)
{
	FaceLayer *face_layer = context;
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->animating = false;

	APP_DEBUG("Animation stopped, finished = %i", finished);
}

void face_layer_animate_in(FaceLayer *face_layer, bool zoom, bool roll)
{
	static const unsigned int duration = 500;
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
		face_layer_animate(duration, delay, AnimationCurveEaseInOut, &radius_anim_impl, roll ? NULL : &handlers, face_layer);
	}
	if(roll) {
		face_layer_animate(2 * duration, delay, AnimationCurveEaseInOut, &roll_anim_impl, &handlers, face_layer);
	}
	if(!zoom && !roll) {
		FaceLayerData *face_layer_data = layer_get_data(face_layer);
		face_layer_data->animating = false;
	}
}
