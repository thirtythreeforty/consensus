#include "preferences.h"

#include "boulder.h"

#include "constants.h"
#include "complications/complication.h"

namespace persist = Boulder::persist;

bool should_vibrate_on_hour()
{
	return persist::load_default<bool>(PERSIST_PREF_VIBRATE_ON_HOUR, true);
}

bool should_vibrate_on_disconnect()
{
	return persist::load_default<bool>(PERSIST_PREF_VIBRATE_ON_DISCONNECT, true);
}

bool should_vibrate_on_connect()
{
	return persist::load_default<bool>(PERSIST_PREF_VIBRATE_ON_CONNECT, false);
}

bool should_hide_no_bluetooth()
{
	return !persist::load_default<bool>(PERSIST_PREF_SHOW_NO_CONNECTION, true);
}

bool should_show_second()
{
	return persist::load_default<bool>(PERSIST_PREF_SHOW_SECOND_HAND, false);
}

bool should_quiet_during_sleep()
{
	return persist::load_default<bool>(PERSIST_PREF_QUIET_DURING_SLEEP, false);
}

complication_config left_complication_type()
{
	return {persist::load_default<unsigned int>(PERSIST_PREF_LEFT_COMPLICATION,
	                                            AbstractComplication::typenum_of<BatteryComplication>()),
	        {
	        	persist::load<unsigned int>(PERSIST_PREF_LEFT_COMPLICATION_OPT1),
	        	persist::load<unsigned int>(PERSIST_PREF_LEFT_COMPLICATION_OPT2),
	        	persist::load<unsigned int>(PERSIST_PREF_LEFT_COMPLICATION_OPT3),
	        	persist::load<unsigned int>(PERSIST_PREF_LEFT_COMPLICATION_OPT4)
	        }};
}

complication_config bottom_complication_type()
{
	return {persist::load_default<unsigned int>(PERSIST_PREF_BOTTOM_COMPLICATION,
	                                            AbstractComplication::typenum_of<WeatherComplication>()),
	        {
	        	persist::load<unsigned int>(PERSIST_PREF_BOTTOM_COMPLICATION_OPT1),
	        	persist::load<unsigned int>(PERSIST_PREF_BOTTOM_COMPLICATION_OPT2),
	        	persist::load<unsigned int>(PERSIST_PREF_BOTTOM_COMPLICATION_OPT3),
	        	persist::load<unsigned int>(PERSIST_PREF_BOTTOM_COMPLICATION_OPT4)
	        }};
}

complication_config right_complication_type()
{
	return {persist::load_default<unsigned int>(PERSIST_PREF_RIGHT_COMPLICATION,
	                                            AbstractComplication::typenum_of<DateComplication>()),
	        {
	        	persist::load<unsigned int>(PERSIST_PREF_RIGHT_COMPLICATION_OPT1),
	        	persist::load<unsigned int>(PERSIST_PREF_RIGHT_COMPLICATION_OPT2),
	        	persist::load<unsigned int>(PERSIST_PREF_RIGHT_COMPLICATION_OPT3),
	        	persist::load<unsigned int>(PERSIST_PREF_RIGHT_COMPLICATION_OPT4)
	        }};
}

uint8_t current_theme()
{
	return persist::load_default<unsigned int>(PERSIST_PREF_THEME, 0);
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

	update_preference<unsigned int>(iterator, KEY_PREF_LEFT_COMPLICATION,
	                                          PERSIST_PREF_LEFT_COMPLICATION);
	update_preference<unsigned int>(iterator, KEY_PREF_BOTTOM_COMPLICATION,
	                                          PERSIST_PREF_BOTTOM_COMPLICATION);
	update_preference<unsigned int>(iterator, KEY_PREF_RIGHT_COMPLICATION,
	                                          PERSIST_PREF_RIGHT_COMPLICATION);

	update_preference<unsigned int>(iterator, KEY_PREF_LEFT_COMPLICATION_OPT1,
	                                          PERSIST_PREF_LEFT_COMPLICATION_OPT1);
	update_preference<unsigned int>(iterator, KEY_PREF_LEFT_COMPLICATION_OPT2,
	                                          PERSIST_PREF_LEFT_COMPLICATION_OPT2);
	update_preference<unsigned int>(iterator, KEY_PREF_LEFT_COMPLICATION_OPT3,
	                                          PERSIST_PREF_LEFT_COMPLICATION_OPT3);
	update_preference<unsigned int>(iterator, KEY_PREF_LEFT_COMPLICATION_OPT4,
	                                          PERSIST_PREF_LEFT_COMPLICATION_OPT4);

	update_preference<unsigned int>(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT1,
	                                          PERSIST_PREF_BOTTOM_COMPLICATION_OPT1);
	update_preference<unsigned int>(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT2,
	                                          PERSIST_PREF_BOTTOM_COMPLICATION_OPT2);
	update_preference<unsigned int>(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT3,
	                                          PERSIST_PREF_BOTTOM_COMPLICATION_OPT3);
	update_preference<unsigned int>(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT4,
	                                          PERSIST_PREF_BOTTOM_COMPLICATION_OPT4);

	update_preference<unsigned int>(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT1,
	                                          PERSIST_PREF_RIGHT_COMPLICATION_OPT1);
	update_preference<unsigned int>(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT2,
	                                          PERSIST_PREF_RIGHT_COMPLICATION_OPT2);
	update_preference<unsigned int>(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT3,
	                                          PERSIST_PREF_RIGHT_COMPLICATION_OPT3);
	update_preference<unsigned int>(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT4,
	                                          PERSIST_PREF_RIGHT_COMPLICATION_OPT4);

	update_preference<unsigned int>(iterator, KEY_PREF_THEME,
	                                          PERSIST_PREF_THEME);

	update_preference<bool>(iterator, KEY_PREF_QUIET_DURING_SLEEP,
	                                  PERSIST_PREF_QUIET_DURING_SLEEP);
}
