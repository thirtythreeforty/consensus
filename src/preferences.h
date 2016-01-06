#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "pebble.h"

bool should_vibrate_on_hour();
bool should_vibrate_on_disconnect();
bool should_hide_no_bluetooth();
bool should_show_second();

void parse_preferences(DictionaryIterator *iterator);

#endif
