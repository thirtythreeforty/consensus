#ifndef FACE_LAYER_H
#define FACE_LAYER_H

extern "C" {
#include <pebble.h>
}

#include "boulder.h"
#include "common.h"
#include "scalable_path.h"

class FaceLayer: public Boulder::Layer
{
	struct hms_t {
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	};

	hms_t requested_time;
	hms_t animation_time;

	bool animating;

	ScalablePath hour_path;
	ScalablePath minute_path;
	ScalablePath second_path;

	bool show_second;

	GColor hour_color;
	GColor minute_color;
	GColor second_color;

public:
	FaceLayer(GRect frame);
	~FaceLayer() = default;

	void set_show_second(bool show);
	void set_time(uint8_t hour, uint8_t min, uint8_t sec);
	void set_colors(GColor hour, GColor min, GColor sec);
	Animation* animate_in(bool zoom, bool roll);

protected:
	void update(GContext* ctx) override;

private:
	static void radius_anim_update(Animation *anim, AnimationProgress dist_normalized);
	static void radius_anim_setup(Animation *anim);

	static void roll_anim_update(Animation *anim, AnimationProgress dist_normalized);

	Animation* make_anim(int duration,
	                     int delay,
	                     AnimationCurve curve,
	                     const AnimationImplementation *implementation,
	                     const AnimationHandlers *handlers);
	Animation* make_zoom_anim();
	Animation* make_roll_anim();

	static void animation_start_handler(Animation *animation, void *context);
	static void animation_stop_handler(Animation *animation, bool finished, void *context);

	static const GPoint hour_hand_path_points[];
	static const GPathInfo hour_hand_path;
	static const GPoint minute_hand_path_points[];
	static const GPathInfo minute_hand_path;

	static const GPoint second_hand_path_points[];
	static const GPathInfo second_hand_path;

};

#endif
