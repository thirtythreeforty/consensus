#include "themes.h"

#include "preferences.h"

namespace {
	// TODO these themes could perhaps save a bit of space if they would use
	// accessor methods; these would be free at worst and optimize away on
	// Aplite where most colors will always be the opposite of the background
	// color.
	Theme themes[] = {
		{ // "Bold"
			._background_color = GColorBlack,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH,
			._tick_color = GColorWhite,
			._tick_resource = RESOURCE_ID_TICKS,
			._hour_hand_color = COLOR_FALLBACK(GColorVividCerulean, GColorWhite),
			._minute_hand_color = GColorWhite,
			._second_hand_color = COLOR_FALLBACK(GColorFolly, GColorWhite),
			._complication_text_color = COLOR_FALLBACK(GColorLightGray, GColorWhite),
			._complication_icon_color = COLOR_FALLBACK(GColorLightGray, GColorWhite),
			._complication_ring_background_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			._battery_complication_color = COLOR_FALLBACK(GColorYellow, GColorWhite),
			._health_complication_color = COLOR_FALLBACK(GColorGreen, GColorWhite),
			._weather_complication_humidity_color = COLOR_FALLBACK(GColorBlue, GColorWhite),
			._weather_complication_temp_color = COLOR_FALLBACK(GColorRed, GColorWhite),
			._complication_fill = false,
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
		},
		{ // "Thin"
			._background_color = GColorBlack,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH,
			._tick_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			._tick_resource = RESOURCE_ID_TICKS,
			._hour_hand_color = COLOR_FALLBACK(GColorVeryLightBlue, GColorWhite),
			._minute_hand_color = COLOR_FALLBACK(GColorPictonBlue, GColorWhite),
			._second_hand_color = COLOR_FALLBACK(GColorRed, GColorWhite),
			._complication_text_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			._complication_icon_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			._complication_ring_background_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			._battery_complication_color = COLOR_FALLBACK(GColorYellow, GColorWhite),
			._health_complication_color = COLOR_FALLBACK(GColorGreen, GColorWhite),
			._weather_complication_humidity_color = COLOR_FALLBACK(GColorBlue, GColorWhite),
			._weather_complication_temp_color = COLOR_FALLBACK(GColorRed, GColorWhite),
			._complication_fill = false,
			._complication_ring_thickness = COLOR_FALLBACK(2, 3),
			._minute_hand_width = 5,
		},
		{ // "Bold White"
			._background_color = GColorWhite,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH_BLACK,
			._tick_color = GColorBlack,
			._tick_resource = RESOURCE_ID_TICKS,
			._hour_hand_color = COLOR_FALLBACK(GColorDukeBlue, GColorBlack),
			._minute_hand_color = COLOR_FALLBACK(GColorBlueMoon, GColorBlack),
			._second_hand_color = COLOR_FALLBACK(GColorDarkCandyAppleRed, GColorBlack),
			._complication_text_color = COLOR_FALLBACK(GColorBlack, GColorBlack),
			._complication_icon_color = COLOR_FALLBACK(GColorBlack, GColorBlack),
			._complication_ring_background_color = COLOR_FALLBACK(GColorDarkGray, GColorBlack),
			._battery_complication_color = COLOR_FALLBACK(GColorChromeYellow, GColorBlack),
			._health_complication_color = COLOR_FALLBACK(GColorDarkGreen, GColorBlack),
			._weather_complication_humidity_color = COLOR_FALLBACK(GColorBlue, GColorBlack),
			._weather_complication_temp_color = COLOR_FALLBACK(GColorRed, GColorBlack),
			._complication_fill = COLOR_FALLBACK(false, false),
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
		},
		{ // "Minimal Dark"
			._background_color = GColorBlack,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH,
			._tick_color = GColorWhite,
			._tick_resource = RESOURCE_ID_TICKS_MINIMAL,
			._hour_hand_color = COLOR_FALLBACK(GColorVividCerulean, GColorWhite),
			._minute_hand_color = GColorWhite,
			._second_hand_color = COLOR_FALLBACK(GColorFolly, GColorWhite),
			._complication_text_color = COLOR_FALLBACK(GColorLightGray, GColorWhite),
			._complication_icon_color = COLOR_FALLBACK(GColorLightGray, GColorWhite),
			._complication_ring_background_color = GColorBlack,
			._battery_complication_color = COLOR_FALLBACK(GColorYellow, GColorWhite),
			._health_complication_color = COLOR_FALLBACK(GColorGreen, GColorWhite),
			._weather_complication_humidity_color = COLOR_FALLBACK(GColorBlue, GColorWhite),
			._weather_complication_temp_color = COLOR_FALLBACK(GColorRed, GColorWhite),
			._complication_fill = false,
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
		},
		{ // "Minimal White"
			._background_color = GColorWhite,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH_BLACK,
			._tick_color = GColorBlack,
			._tick_resource = RESOURCE_ID_TICKS_MINIMAL,
			._hour_hand_color = COLOR_FALLBACK(GColorDukeBlue, GColorBlack),
			._minute_hand_color = COLOR_FALLBACK(GColorBlueMoon, GColorBlack),
			._second_hand_color = COLOR_FALLBACK(GColorDarkCandyAppleRed, GColorBlack),
			._complication_text_color = COLOR_FALLBACK(GColorBlack, GColorBlack),
			._complication_icon_color = COLOR_FALLBACK(GColorBlack, GColorBlack),
			._complication_ring_background_color = GColorWhite,
			._battery_complication_color = COLOR_FALLBACK(GColorChromeYellow, GColorBlack),
			._health_complication_color = COLOR_FALLBACK(GColorDarkGreen, GColorBlack),
			._weather_complication_humidity_color = COLOR_FALLBACK(GColorBlue, GColorBlack),
			._weather_complication_temp_color = COLOR_FALLBACK(GColorRed, GColorBlack),
			._complication_fill = COLOR_FALLBACK(false, false),
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
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
