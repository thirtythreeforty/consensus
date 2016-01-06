#include "preferences.h"

#include "constants.h"

static bool persist_read_bool_default(uint32_t key, bool default_value)
{
	return persist_exists(key)
		? persist_read_bool(key)
		: default_value;
}

bool should_vibrate_on_hour()
{
	return persist_read_bool_default(PERSIST_PREF_VIBRATE_ON_HOUR, true);
}

bool should_vibrate_on_disconnect()
{
	return persist_read_bool_default(PERSIST_PREF_VIBRATE_ON_DISCONNECT, true);
}

bool should_hide_no_bluetooth()
{
	return !persist_read_bool_default(PERSIST_PREF_SHOW_NO_CONNECTION, true);
}

bool should_show_second()
{
	return persist_read_bool_default(KEY_PREF_SHOW_SECOND_HAND, false);
}

static void update_bool_preference(DictionaryIterator *iter, uint32_t dict_key, uint32_t persist_key)
{
	Tuple *tuple = dict_find(iter, dict_key);
	if(tuple) {
		persist_write_bool(persist_key, tuple->value->uint8);
	}
}

void parse_preferences(DictionaryIterator *iterator)
{
	update_bool_preference(iterator, KEY_PREF_SHOW_SECOND_HAND,
	                                 PERSIST_PREF_SHOW_SECOND_HAND);
	update_bool_preference(iterator, KEY_PREF_SHOW_NO_CONNECTION,
	                                 PERSIST_PREF_SHOW_NO_CONNECTION);

	update_bool_preference(iterator, KEY_PREF_VIBRATE_ON_HOUR,
	                                 PERSIST_PREF_VIBRATE_ON_HOUR);
	update_bool_preference(iterator, KEY_PREF_VIBRATE_ON_DISCONNECT,
	                                 PERSIST_PREF_VIBRATE_ON_DISCONNECT);
}
