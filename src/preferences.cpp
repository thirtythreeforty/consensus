#include "preferences.h"

#include "boulder.h"

#include "constants.h"

namespace persist = Boulder::persist;

bool should_vibrate_on_hour()
{
	return persist::load_default<bool>(PERSIST_PREF_VIBRATE_ON_HOUR, true);
}

bool should_vibrate_on_disconnect()
{
	return persist::load_default<bool>(PERSIST_PREF_VIBRATE_ON_DISCONNECT, true);
}

bool should_hide_no_bluetooth()
{
	return !persist::load_default<bool>(PERSIST_PREF_SHOW_NO_CONNECTION, true);
}

bool should_show_second()
{
	return persist::load_default<bool>(PERSIST_PREF_SHOW_SECOND_HAND, false);
}

template<typename T>
void update_preference(DictionaryIterator *iter, uint32_t dict_key, uint32_t persist_key)
{
	Boulder::Tuple tuple = dict_find(iter, dict_key);
	if(tuple.valid()) {
		persist::save(persist_key, tuple.value<T>());
	}
}

void parse_preferences(DictionaryIterator *iterator)
{
	update_preference<bool>(iterator, KEY_PREF_SHOW_SECOND_HAND,
	                                  PERSIST_PREF_SHOW_SECOND_HAND);
	update_preference<bool>(iterator, KEY_PREF_SHOW_NO_CONNECTION,
	                                  PERSIST_PREF_SHOW_NO_CONNECTION);

	update_preference<bool>(iterator, KEY_PREF_VIBRATE_ON_HOUR,
	                                  PERSIST_PREF_VIBRATE_ON_HOUR);
	update_preference<bool>(iterator, KEY_PREF_VIBRATE_ON_DISCONNECT,
	                                  PERSIST_PREF_VIBRATE_ON_DISCONNECT);
}
