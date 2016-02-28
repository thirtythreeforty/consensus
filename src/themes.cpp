#include "themes.h"

#include "preferences.h"

namespace {
	// TODO these themes could perhaps save a bit of space if they would use
	// accessor methods; these would be free at worst and optimize away on
	// Aplite where most colors will always be the opposite of the background
	// color.
	Theme themes[] = {
		{ // "Bold"
			.background_color = GColorBlack,
			.tick_color = GColorWhite,
			.hour_hand_color = COLOR_FALLBACK(GColorVividCerulean, GColorWhite),
			.minute_hand_color = GColorWhite,
			.second_hand_color = COLOR_FALLBACK(GColorFolly, GColorWhite),
			.complication_text_color = COLOR_FALLBACK(GColorLightGray, GColorWhite),
			.complication_icon_color = COLOR_FALLBACK(GColorLightGray, GColorWhite),
			.complication_ring_background_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			.battery_complication_color = COLOR_FALLBACK(GColorYellow, GColorWhite),
			.health_complication_color = COLOR_FALLBACK(GColorGreen, GColorWhite),
			.complication_fill = false,
			.complication_ring_thickness = 3,
			.minute_hand_width = 7,
		},
		{ // "Thin"
			.background_color = GColorBlack,
			.tick_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			.hour_hand_color = COLOR_FALLBACK(GColorVeryLightBlue, GColorWhite),
			.minute_hand_color = COLOR_FALLBACK(GColorPictonBlue, GColorWhite),
			.second_hand_color = COLOR_FALLBACK(GColorRed, GColorWhite),
			.complication_text_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			.complication_icon_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			.complication_ring_background_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			.battery_complication_color = COLOR_FALLBACK(GColorYellow, GColorWhite),
			.health_complication_color = COLOR_FALLBACK(GColorGreen, GColorWhite),
			.complication_fill = false,
			.complication_ring_thickness = COLOR_FALLBACK(2, 3),
			.minute_hand_width = 5,
		},
		{ // "White"
			.background_color = GColorWhite,
			.tick_color = GColorBlack,
			.hour_hand_color = COLOR_FALLBACK(GColorDukeBlue, GColorBlack),
			.minute_hand_color = COLOR_FALLBACK(GColorBlueMoon, GColorBlack),
			.second_hand_color = COLOR_FALLBACK(GColorDarkCandyAppleRed, GColorBlack),
			.complication_text_color = COLOR_FALLBACK(GColorBlack, GColorBlack),
			.complication_icon_color = COLOR_FALLBACK(GColorBlack, GColorBlack),
			.complication_ring_background_color = COLOR_FALLBACK(GColorDarkGray, GColorBlack),
			.battery_complication_color = COLOR_FALLBACK(GColorChromeYellow, GColorBlack),
			.health_complication_color = COLOR_FALLBACK(GColorDarkGreen, GColorBlack),
			.complication_fill = COLOR_FALLBACK(false, false),
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
