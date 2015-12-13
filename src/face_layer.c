#include "face_layer.h"

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

struct st_face_layer_data {
	struct tm requested_time;

	bool animating;

	GPath *hour_path;
	GPath *minute_path;

	GColor hour_color;
	GColor minute_color;
};
typedef struct st_face_layer_data FaceLayerData;

void face_layer_redraw(Layer *layer, GContext *ctx)
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

FaceLayer *face_layer_create(void)
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

	FaceLayer *layer = layer_create_with_data(GRect(0, 0, 144, 168), sizeof(FaceLayerData));
	layer_set_update_proc(layer, face_layer_redraw);

	FaceLayerData *face_layer_data = layer_get_data(layer);

	face_layer_data->animating = false;

	GRect bounds = layer_get_bounds(layer);
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

void face_layer_set_time(FaceLayer *face_layer, struct tm *time)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->requested_time = *time;

	int local_hour = time->tm_hour % 12;

	unsigned int hour_angle = TRIG_MAX_ANGLE * local_hour / 12 +
	                          TRIG_MAX_ANGLE * face_layer_data->requested_time.tm_min / (60 * 60);

	unsigned int minute_angle = (TRIG_MAX_ANGLE *
	                             (face_layer_data->requested_time.tm_min * 60 +
	                              face_layer_data->requested_time.tm_sec))
	                            / (60 * 60);

	APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE,
	        "(hour, h_angle) = (%i, %i); (minute, m_angle) = (%i, %i)",
	        local_hour, hour_angle,
	        face_layer_data->requested_time.tm_min, minute_angle);

	gpath_rotate_to(face_layer_data->hour_path, hour_angle);
	gpath_rotate_to(face_layer_data->minute_path, minute_angle);

	layer_mark_dirty(face_layer);
}

void face_layer_set_colors(FaceLayer *face_layer, GColor hour, GColor minute)
{
	FaceLayerData *face_layer_data = layer_get_data(face_layer);

	face_layer_data->hour_color = hour;
	face_layer_data->minute_color = minute;
}

void face_layer_animate_in(void)
{
}
