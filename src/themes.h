#ifndef THEMES_H
#define THEMES_H

extern "C" {
#include "pebble.h"
}

#define THEME_ACCESSOR(Type, ElementName) \
	Type _ ## ElementName; \
	const auto& ElementName() const { return _ ## ElementName; }

struct Theme
{
	THEME_ACCESSOR(GColor, background_color);
	THEME_ACCESSOR(uint8_t, no_bluetooth_resource);
	THEME_ACCESSOR(GColor, tick_color);
	THEME_ACCESSOR(uint8_t, tick_resource);
	THEME_ACCESSOR(GColor, hour_hand_color);
	THEME_ACCESSOR(GColor, minute_hand_color);
	THEME_ACCESSOR(GColor, second_hand_color);
	THEME_ACCESSOR(GColor, complication_text_color);
	THEME_ACCESSOR(GColor, complication_icon_color);
	THEME_ACCESSOR(GColor, complication_ring_background_color);
	THEME_ACCESSOR(GColor, battery_complication_color);
	THEME_ACCESSOR(GColor, health_complication_color);
	THEME_ACCESSOR(GColor, weather_complication_humidity_color);
	THEME_ACCESSOR(GColor, weather_complication_temp_color);
	THEME_ACCESSOR(bool, complication_fill);
	THEME_ACCESSOR(uint8_t, complication_ring_thickness);
	THEME_ACCESSOR(uint8_t, minute_hand_width);
};

#undef THEME_ACCESSOR

const Theme& theme();
void set_theme();

#endif
