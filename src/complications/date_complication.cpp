#include "complication_common.h"

#include "common.h"

typedef struct {
	bool animating;
	uint8_t anim_frames_skipped;

	uint8_t requested_date;

	TextLayer *date_layer;
	char date_layer_text[3];
} DateComplicationData;

void DateComplication::update(GContext *ctx)
{
	base_complication_update_1(*this, ctx, GColorClear, 0);
}

DateComplication::DateComplication(GRect frame)
	: Complication(frame)
	, animating(true)
	, date_layer(calculate_date_frame())
	, date_layer_text({0})
{
	animating = true; // Must call date_complication_animate_in

	date_layer.set_background_color(GColorClear);
	date_layer.set_text_color(GColorDarkGray);
	date_layer.set_text_alignment(GTextAlignmentCenter);
	date_layer.set_font(fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
	date_layer.set_text(date_layer_text.data());
	this->add_child(date_layer);
}

GRect DateComplication::calculate_date_frame()
{
	// As far as I can tell this is just a magic number.  3px margins per side?
	static const unsigned int text_margins = 6;
	static const unsigned int date_font_height = 20;
	static const unsigned int date_layer_height = text_margins + date_font_height;
	GRect date_frame = this->get_bounds();
	date_frame.origin.y = date_frame.size.h / 2 - date_layer_height / 2;
	date_frame.size.h = date_layer_height;

	return date_frame;
}

void DateComplication::set_displayed(uint8_t mday)
{
	snprintf(date_layer_text.data(), date_layer_text.size(), "%02u", mday);
	mark_dirty();
}

void DateComplication::time_changed(struct tm *time)
{
	if(animating) {
		requested_date = time->tm_mday;
	}
	else {
		set_displayed(time->tm_mday);
	}
}

void DateComplication::spin_animation_update(Animation* anim, AnimationProgress progress)
{
	auto complication = static_cast<DateComplication*>(animation_get_context(anim));

	if(complication->anim_frames_skipped == 1) {
		uint8_t random_date = rand() % 100;
		complication->set_displayed(random_date);
		complication->anim_frames_skipped = 0;
	}
	else {
		complication->anim_frames_skipped++;
	}
}

void DateComplication::spin_animation_started(Animation *animation, void *context)
{
	auto complication = static_cast<DateComplication*>(context);
	complication->animating = true;
	complication->anim_frames_skipped = 0;
}

void DateComplication::spin_animation_stopped(Animation *animation, bool finished, void *context)
{
	auto complication = static_cast<DateComplication*>(context);
	complication->animating = false;
	complication->set_displayed(complication->requested_date);
}

Animation* DateComplication::animate_in()
{
	static const AnimationImplementation date_spinup_anim_impl = {
		.setup = NULL,
		.update = DateComplication::spin_animation_update,
		.teardown = NULL
	};
	static const AnimationHandlers date_spinup_anim_handlers = {
		.started = DateComplication::spin_animation_started,
		.stopped = DateComplication::spin_animation_stopped
	};

	return base_complication_animate_in(&date_spinup_anim_impl,
	                                    &date_spinup_anim_handlers,
	                                    this);
}

