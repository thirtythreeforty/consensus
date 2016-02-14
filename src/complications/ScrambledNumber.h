#ifndef SCRAMBLEDNUMBER_H
#define SCRAMBLEDNUMBER_H

extern "C" {
#include <pebble.h>
}

#include <array>

#include "boulder.h"

class ScrambledNumber
{
	Boulder::TextLayer text_layer;
	std::array<char, 6> text_layer_text;
	const char *format_string;

	int32_t requested_number;
	enum {
		INIT,
		STOPPED,
		ANIMATING
	} state;
	uint8_t anim_frames_skipped;

public:
	explicit ScrambledNumber(GRect frame, const char* format_string);
	~ScrambledNumber() = default;

	Boulder::TextLayer& get_text_layer() { return text_layer; }

	void reconfigure_color();

	void set(int32_t number);
	void set_format(const char* fmt, int32_t number);

private:
	GRect calculate_text_frame(const GRect& our_frame);
	void format(int32_t display_number);

	void animate_to_requested();

	static void scramble_animation_stopped(Animation *animation, bool finished, void *context);
	static void scramble_animation_update(Animation* anim, AnimationProgress progress);
};

#endif
