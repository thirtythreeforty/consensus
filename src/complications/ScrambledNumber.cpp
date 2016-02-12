#include "ScrambledNumber.h"

#include "themes.h"

ScrambledNumber::ScrambledNumber(GRect frame, const char* format_string)
	: text_layer(calculate_text_frame(frame))
	, text_layer_text({0})
	, format_string(format_string)
	, state(INIT)
{
	text_layer.set_text_color(theme().complication_text_color);
	text_layer.set_background_color(GColorClear);
	text_layer.set_text(text_layer_text.data());
	text_layer.set_text_alignment(GTextAlignmentCenter);
	text_layer.set_font(fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
}

GRect ScrambledNumber::calculate_text_frame(const GRect& our_frame)
{
	// As far as I can tell this is just a magic number.  3px margins per side?
	static const unsigned int text_margins = 6;
	static const unsigned int text_font_height = 20;
	static const unsigned int text_layer_height = text_margins + text_font_height;

	GRect text_frame = our_frame;
	text_frame.origin.y = text_frame.size.h / 2 - text_layer_height / 2;
	text_frame.size.h = text_layer_height;

	return text_frame;
}

void ScrambledNumber::set(int32_t number)
{
	requested_number = number;
	if(state != ANIMATING) {
		const bool is_close = abs(number - requested_number) <= 5;
		if(state == INIT || !is_close) {
			animate_to_requested();
		}
		else { // STATE == STOPPED && is_close
			format(number);
		}
	}
}

void ScrambledNumber::set_format(const char *fmt, int32_t number)
{
	if(state != ANIMATING) {
		state = INIT;
	}
	format_string = fmt;
	set(number);
}

void ScrambledNumber::format(int32_t display_number)
{
	snprintf(text_layer_text.data(), text_layer_text.size(), format_string, display_number);
	// need this form because Boulder does not yet have the concept of a LayerRef
	layer_mark_dirty(text_layer);
}

void ScrambledNumber::scramble_animation_update(Animation* anim, AnimationProgress progress)
{
	auto scrambled_number = static_cast<ScrambledNumber*>(animation_get_context(anim));

	if(scrambled_number->anim_frames_skipped == 1) {
		uint8_t random_number = rand() % 100;
		scrambled_number->format(random_number);
		scrambled_number->anim_frames_skipped = 0;
	}
	else {
		scrambled_number->anim_frames_skipped++;
	}
}

void ScrambledNumber::scramble_animation_stopped(Animation *animation, bool finished, void *context)
{
	auto scrambled_number = static_cast<ScrambledNumber*>(context);
	scrambled_number->state = STOPPED;
	scrambled_number->format(scrambled_number->requested_number);
}

void ScrambledNumber::animate_to_requested()
{
	static const AnimationImplementation scramble_anim_impl = {
		.setup = NULL,
		.update = ScrambledNumber::scramble_animation_update,
		.teardown = NULL
	};
	static const AnimationHandlers scramble_anim_handlers = {
		.started = NULL,
		.stopped = ScrambledNumber::scramble_animation_stopped
	};

	state = ANIMATING;
	anim_frames_skipped = 0;

	Animation *anim = animation_create();
	animation_set_implementation(anim, &scramble_anim_impl);

	static const unsigned int duration = 700;
	static const unsigned int delay = 200;

	animation_set_duration(anim, duration);
	animation_set_delay(anim, delay);
	animation_set_curve(anim, AnimationCurveEaseInOut);
	animation_set_handlers(anim, scramble_anim_handlers, this);

	animation_schedule(anim);
}
