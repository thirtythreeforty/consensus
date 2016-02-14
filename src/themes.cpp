#include "themes.h"

#include "preferences.h"

namespace {
	Theme themes[] = {
		{ // "Bold"
			.background_color = GColorBlack,
			.hour_hand_color = GColorVividCerulean,
			.minute_hand_color = GColorWhite,
			.second_hand_color = GColorFolly,
			.complication_text_color = GColorLightGray,
			.complication_icon_color = GColorLightGray,
			.complication_ring_background_color = GColorDarkGray,
			.complication_ring_thickness = 3,
			.minute_hand_width = 7,
		},
		{ // "Thin"
			.background_color = GColorBlack,
			.hour_hand_color = GColorVeryLightBlue,
			.minute_hand_color = GColorPictonBlue,
			.second_hand_color = GColorRed,
			.complication_text_color = GColorDarkGray,
			.complication_icon_color = GColorDarkGray,
			.complication_ring_background_color = GColorDarkGray,
			.complication_ring_thickness = 2,
			.minute_hand_width = 5,
		},
		{ // "White"
			.background_color = GColorWhite,
			.hour_hand_color = GColorOxfordBlue,
			.minute_hand_color = GColorBlack,
			.second_hand_color = GColorDarkCandyAppleRed,
			.complication_text_color = GColorDarkGray,
			.complication_icon_color = GColorDarkGray,
			.complication_ring_background_color = GColorLightGray,
			.complication_ring_thickness = 3,
			.minute_hand_width = 7,
		},
	};
	const Theme* _theme; // "Bold" default theme
}

const Theme& theme()
{
	return *_theme;
}

void set_theme()
{
	_theme = &themes[current_theme()];
}
