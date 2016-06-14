#ifndef FACE_LAYER_H
#define FACE_LAYER_H

extern "C" {
#include <pebble.h>
}

#include "complications/animated.h"
#include "boulder.h"
#include "common.h"
#include "scalable_path.h"

class FaceLayer final: public Boulder::Layer
{
	class Hand final: private AnimatedCallback
	{
		using angle_t = int32_t;

		static inline bool is_close(const int32_t& a, const int32_t& b) {
			constexpr int32_t Increment = 1.1 * ANIMATION_NORMALIZED_MAX / 60;

			return abs(a - b) <= Increment
			       // Handle wraparound from 59 to 0:
			       || abs(a - b) >= (ANIMATION_NORMALIZED_MAX - Increment);
		}

		Animated<int32_t, is_close> angle;
		Animated<int32_t> scale;
		ScalablePath path;

		Boulder::Layer& layer;

	public:
		Hand(Boulder::Layer& layer, const GPathInfo *path_info, GPoint center);

		inline void set_angle(angle_t a) { angle = a; }
		void zoom(bool in);

		const ScalablePath& get_path() const { return path; }

	private:
		virtual void on_animated_update(void *animated) override;
	};

	Hand hour_hand, min_hand, sec_hand;

	bool large;

public:
	FaceLayer(GRect frame, bool large);
	~FaceLayer() = default;

	void set_show_second(bool show);
	void set_time(uint8_t hour, uint8_t min, uint8_t sec);

protected:
	void update(GContext* ctx) override;

private:
	static void radius_anim_update(Animation *anim, AnimationProgress dist_normalized);
	static void radius_anim_setup(Animation *anim);

	static void roll_anim_update(Animation *anim, AnimationProgress dist_normalized);
	static void roll_anim_setup(Animation *anim);

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

	static const GPoint small_hour_hand_path_points[];
	static const GPathInfo small_hour_hand_path;
	static const GPoint small_minute_hand_path_points[];
	static const GPathInfo small_minute_hand_path;
	static const GPoint small_second_hand_path_points[];
	static const GPathInfo small_second_hand_path;
};

#endif
