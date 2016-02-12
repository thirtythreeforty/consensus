#ifndef THEMES_H
#define THEMES_H

extern "C" {
#include "pebble.h"
}

struct Theme
{
	GColor background_color;
	GColor hour_hand_color;
	GColor minute_hand_color;
	GColor second_hand_color;
	GColor complication_text_color;
	GColor complication_icon_color;
	GColor complication_ring_background_color;
	uint8_t complication_ring_thickness;
	uint8_t minute_hand_width;
};

const Theme& theme();
void setTheme();

#endif
