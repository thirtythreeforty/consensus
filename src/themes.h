#ifndef THEMES_H
#define THEMES_H

extern "C" {
#include "pebble.h"
}

#define THEME_ACCESSOR(Type, ElementName) \
	Type _ ## ElementName; \
	const auto& ElementName() const { return _ ## ElementName; }

#ifdef PBL_COLOR
#define THEME_ACCESSOR_FGCOLOR(ElementName) \
	THEME_ACCESSOR(GColor, ElementName)
#else
#define THEME_ACCESSOR_FGCOLOR(ElementName) \
	const auto& ElementName() const { return _tick_color; }
#endif

struct Theme
{
	THEME_ACCESSOR(GColor, background_color);
	THEME_ACCESSOR(uint8_t, no_bluetooth_resource);
	THEME_ACCESSOR(GColor, tick_color);
	THEME_ACCESSOR(uint8_t, tick_resource);
	THEME_ACCESSOR(bool, complication_fill);
	THEME_ACCESSOR(uint8_t, complication_ring_thickness);
	THEME_ACCESSOR(uint8_t, minute_hand_width);
	THEME_ACCESSOR_FGCOLOR(hour_hand_color);
	THEME_ACCESSOR_FGCOLOR(minute_hand_color);
	THEME_ACCESSOR_FGCOLOR(second_hand_color);
	THEME_ACCESSOR_FGCOLOR(complication_text_color);
	THEME_ACCESSOR_FGCOLOR(complication_icon_color);
	THEME_ACCESSOR_FGCOLOR(complication_ring_background_color);
	THEME_ACCESSOR_FGCOLOR(battery_complication_color);
	THEME_ACCESSOR_FGCOLOR(health_complication_color);
	THEME_ACCESSOR_FGCOLOR(weather_complication_humidity_color);
	THEME_ACCESSOR_FGCOLOR(weather_complication_temp_color);
};

#undef THEME_ACCESSOR

const Theme& theme();
void set_theme();

#endif
