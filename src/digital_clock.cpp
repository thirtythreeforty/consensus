#include "digital_clock.h"

#include "preferences.h"

DigitalClock::DigitalClock(GRect frame)
	: TimeCallback(should_show_second() ? SECOND_UNIT : MINUTE_UNIT)
	, text_layer(frame)
{
	text_layer.set_text_alignment(GTextAlignmentCenter);
	text_layer.set_font(fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
}

void DigitalClock::configure()
{
}

void DigitalClock::on_tick(struct tm*, TimeUnits)
{
}

void DigitalClock::set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
}
