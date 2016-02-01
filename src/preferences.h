#ifndef PREFERENCES_H
#define PREFERENCES_H

extern "C" {
#include "pebble.h"
}

bool should_vibrate_on_hour();
bool should_vibrate_on_disconnect();
bool should_hide_no_bluetooth();
bool should_show_second();

unsigned int left_complication_type();
unsigned int bottom_complication_type();
unsigned int right_complication_type();

void parse_preferences(DictionaryIterator *iterator);

#endif
