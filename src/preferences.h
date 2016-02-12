#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <array>
#include <utility>

extern "C" {
#include "pebble.h"
}

bool should_vibrate_on_hour();
bool should_vibrate_on_disconnect();
bool should_vibrate_on_connect();
bool should_hide_no_bluetooth();
bool should_show_second();

uint8_t current_theme();

using complication_config = std::pair<unsigned int, std::array<unsigned int, 4>>;

complication_config left_complication_type();
complication_config bottom_complication_type();
complication_config right_complication_type();

void parse_preferences(DictionaryIterator *iterator);

#endif
