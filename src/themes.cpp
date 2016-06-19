#include "themes.h"

#include "preferences.h"

namespace {
	Theme themes[] = {
		{ // "Bold"
			._background_color = GColorBlack,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH,
			._low_batt_resource = RESOURCE_ID_LOW_BATTERY,
			._chg_batt_resource = RESOURCE_ID_CHG_BATTERY,
			._tick_color = GColorWhite,
			._tick_resource = RESOURCE_ID_TICKS,
			._complication_fill = false,
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
#ifdef PBL_COLOR
			._hour_hand_color = GColorVividCerulean,
			._minute_hand_color = GColorWhite,
			._second_hand_color = GColorFolly,
			._complication_text_color = GColorLightGray,
			._complication_icon_color = GColorLightGray,
			._complication_ring_background_color = GColorDarkGray,
			._battery_complication_color = GColorYellow,
			._health_complication_color = GColorGreen,
			._health_complication_dot_color = GColorYellow,
			._weather_complication_humidity_color = GColorBlue,
			._weather_complication_temp_color = GColorRed,
#endif
		},
		{ // "Thin"
			._background_color = GColorBlack,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH,
			._low_batt_resource = RESOURCE_ID_LOW_BATTERY,
			._chg_batt_resource = RESOURCE_ID_CHG_BATTERY,
			._tick_color = COLOR_FALLBACK(GColorDarkGray, GColorWhite),
			._tick_resource = RESOURCE_ID_TICKS,
			._complication_fill = false,
			._complication_ring_thickness = COLOR_FALLBACK(2, 3),
			._minute_hand_width = 5,
#ifdef PBL_COLOR
			._hour_hand_color = GColorVeryLightBlue,
			._minute_hand_color = GColorPictonBlue,
			._second_hand_color = GColorRed,
			._complication_text_color = GColorDarkGray,
			._complication_icon_color = GColorDarkGray,
			._complication_ring_background_color = GColorDarkGray,
			._battery_complication_color = GColorYellow,
			._health_complication_color = GColorGreen,
			._health_complication_dot_color = GColorYellow,
			._weather_complication_humidity_color = GColorBlue,
			._weather_complication_temp_color = GColorRed,
#endif
		},
		{ // "Bold White"
			._background_color = GColorWhite,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH_BLACK,
			._low_batt_resource = RESOURCE_ID_LOW_BATTERY_BLACK,
			._chg_batt_resource = RESOURCE_ID_CHG_BATTERY_BLACK,
			._tick_color = GColorBlack,
			._tick_resource = RESOURCE_ID_TICKS,
			._complication_fill = false,
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
#ifdef PBL_COLOR
			._hour_hand_color = GColorDukeBlue,
			._minute_hand_color = GColorBlueMoon,
			._second_hand_color = GColorDarkCandyAppleRed,
			._complication_text_color = GColorBlack,
			._complication_icon_color = GColorBlack,
			._complication_ring_background_color = GColorDarkGray,
			._battery_complication_color = GColorChromeYellow,
			._health_complication_color = GColorDarkGreen,
			._health_complication_dot_color = GColorChromeYellow,
			._weather_complication_humidity_color = GColorBlue,
			._weather_complication_temp_color = GColorRed,
#endif
		},
		{ // "Minimal Dark"
			._background_color = GColorBlack,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH,
			._low_batt_resource = RESOURCE_ID_LOW_BATTERY,
			._chg_batt_resource = RESOURCE_ID_CHG_BATTERY,
			._tick_color = GColorWhite,
			._tick_resource = RESOURCE_ID_TICKS_MINIMAL,
			._complication_fill = false,
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
#ifdef PBL_COLOR
			._hour_hand_color = GColorVividCerulean,
			._minute_hand_color = GColorWhite,
			._second_hand_color = GColorFolly,
			._complication_text_color = GColorLightGray,
			._complication_icon_color = GColorLightGray,
			._complication_ring_background_color = GColorBlack,
			._battery_complication_color = GColorYellow,
			._health_complication_color = GColorGreen,
			._health_complication_dot_color = GColorYellow,
			._weather_complication_humidity_color = GColorBlue,
			._weather_complication_temp_color = GColorRed,
#endif
		},
		{ // "Minimal White"
			._background_color = GColorWhite,
			._no_bluetooth_resource = RESOURCE_ID_NO_BLUETOOTH_BLACK,
			._low_batt_resource = RESOURCE_ID_LOW_BATTERY_BLACK,
			._chg_batt_resource = RESOURCE_ID_CHG_BATTERY_BLACK,
			._tick_color = GColorBlack,
			._tick_resource = RESOURCE_ID_TICKS_MINIMAL,
			._complication_fill = false,
			._complication_ring_thickness = 3,
			._minute_hand_width = 7,
#ifdef PBL_COLOR
			._hour_hand_color = GColorDukeBlue,
			._minute_hand_color = GColorBlueMoon,
			._second_hand_color = GColorDarkCandyAppleRed,
			._complication_text_color = GColorBlack,
			._complication_icon_color = GColorBlack,
			._complication_ring_background_color = GColorWhite,
			._battery_complication_color = GColorChromeYellow,
			._health_complication_color = GColorDarkGreen,
			._health_complication_dot_color = GColorChromeYellow,
			._weather_complication_humidity_color = GColorBlue,
			._weather_complication_temp_color = GColorRed,
#endif
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
