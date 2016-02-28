#include "preferences.h"

#include "boulder.h"

#include "constants.h"
#include "complications/complication.h"

namespace persist = Boulder::persist;

struct Preferences {
	uint8_t prefs_version = 2;

	uint8_t theme = 0;

	bool should_vibrate_on_hour = true;
	bool should_vibrate_on_disconnect = true;
	bool should_vibrate_on_connect = false;
	bool should_show_no_connection = true;
	bool should_show_second = false;
	bool should_quiet_during_sleep = false;

	uint8_t left_complication_type = AbstractComplication::typenum_of<BatteryComplication>();
	uint32_t left_complication_opt1 = 0;
	uint32_t left_complication_opt2 = 0;
	uint32_t left_complication_opt3 = 0;
	uint32_t left_complication_opt4 = 0;

	uint8_t bottom_complication_type = AbstractComplication::typenum_of<WeatherComplication>();
	uint32_t bottom_complication_opt1 = 0;
	uint32_t bottom_complication_opt2 = 0;
	uint32_t bottom_complication_opt3 = 0;
	uint32_t bottom_complication_opt4 = 0;

	uint8_t right_complication_type = AbstractComplication::typenum_of<DateComplication>();
	uint32_t right_complication_opt1 = 0;
	uint32_t right_complication_opt2 = 0;
	uint32_t right_complication_opt3 = 0;
	uint32_t right_complication_opt4 = 0;
};

static Preferences prefs;

bool should_vibrate_on_hour()
{
	return prefs.should_vibrate_on_hour;
}

bool should_vibrate_on_disconnect()
{
	return prefs.should_vibrate_on_disconnect;
}

bool should_vibrate_on_connect()
{
	return prefs.should_vibrate_on_connect;
}

bool should_hide_no_bluetooth()
{
	return !prefs.should_show_no_connection;
}

bool should_show_second()
{
	return prefs.should_show_second;
}

bool should_quiet_during_sleep()
{
	return prefs.should_quiet_during_sleep;
}

complication_config left_complication_type()
{
	return {
		prefs.left_complication_type,
		{
			prefs.left_complication_opt1,
			prefs.left_complication_opt2,
			prefs.left_complication_opt3,
			prefs.left_complication_opt4,
		}
	};
}

complication_config bottom_complication_type()
{
	return {
		prefs.bottom_complication_type,
		{
			prefs.bottom_complication_opt1,
			prefs.bottom_complication_opt2,
			prefs.bottom_complication_opt3,
			prefs.bottom_complication_opt4,
		}
	};
}

complication_config right_complication_type()
{
	return {
		prefs.right_complication_type,
		{
			prefs.right_complication_opt1,
			prefs.right_complication_opt2,
			prefs.right_complication_opt3,
			prefs.right_complication_opt4,
		}
	};
}

uint8_t current_theme()
{
	return prefs.theme;
}

template<typename T>
void update_preference(DictionaryIterator *iter, uint32_t dict_key, T& pref)
{
	Boulder::Tuple tuple = dict_find(iter, dict_key);
	if(tuple.valid()) {
		pref = tuple.value<T>();
	}
}

static void save_preferences()
{
	persist::save_data(PERSIST_PREFS_STRUCT, prefs);
}

static void load_preferences()
{
	persist::load_data(PERSIST_PREFS_STRUCT, prefs);
}

void parse_preferences(DictionaryIterator *iterator)
{
	update_preference(iterator, KEY_PREF_SHOW_SECOND_HAND, prefs.should_show_second);
	update_preference(iterator, KEY_PREF_SHOW_NO_CONNECTION, prefs.should_show_no_connection);

	update_preference(iterator, KEY_PREF_VIBRATE_ON_HOUR, prefs.should_vibrate_on_hour);
	update_preference(iterator, KEY_PREF_VIBRATE_ON_DISCONNECT, prefs.should_vibrate_on_disconnect);
	update_preference(iterator, KEY_PREF_VIBRATE_ON_CONNECT, prefs.should_vibrate_on_connect);

	update_preference(iterator, KEY_PREF_LEFT_COMPLICATION, prefs.left_complication_type);
	update_preference(iterator, KEY_PREF_BOTTOM_COMPLICATION, prefs.bottom_complication_type);
	update_preference(iterator, KEY_PREF_RIGHT_COMPLICATION, prefs.right_complication_type);

	update_preference(iterator, KEY_PREF_QUIET_DURING_SLEEP, prefs.should_quiet_during_sleep);

	update_preference(iterator, KEY_PREF_LEFT_COMPLICATION_OPT1, prefs.left_complication_opt1);
	update_preference(iterator, KEY_PREF_LEFT_COMPLICATION_OPT2, prefs.left_complication_opt2);
	update_preference(iterator, KEY_PREF_LEFT_COMPLICATION_OPT3, prefs.left_complication_opt3);
	update_preference(iterator, KEY_PREF_LEFT_COMPLICATION_OPT4, prefs.left_complication_opt4);

	update_preference(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT1, prefs.bottom_complication_opt1);
	update_preference(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT2, prefs.bottom_complication_opt2);
	update_preference(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT3, prefs.bottom_complication_opt3);
	update_preference(iterator, KEY_PREF_BOTTOM_COMPLICATION_OPT4, prefs.bottom_complication_opt4);

	update_preference(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT1, prefs.right_complication_opt1);
	update_preference(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT2, prefs.right_complication_opt2);
	update_preference(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT3, prefs.right_complication_opt3);
	update_preference(iterator, KEY_PREF_RIGHT_COMPLICATION_OPT4, prefs.right_complication_opt4);

	update_preference(iterator, KEY_PREF_THEME, prefs.theme);

	save_preferences();
}

template<typename T>
static void read_legacy_pref(uint32_t persist_key, T& pref)
{
	if(persist::exists(persist_key)) {
		pref = persist::load<T>(persist_key);
	}
}

static void upgrade_legacy_preferences()
{
	read_legacy_pref(PERSIST_PREF_SHOW_SECOND_HAND_DEPRECATED, prefs.should_show_second);
	read_legacy_pref(PERSIST_PREF_SHOW_NO_CONNECTION_DEPRECATED, prefs.should_show_no_connection);

	read_legacy_pref(PERSIST_PREF_VIBRATE_ON_HOUR_DEPRECATED, prefs.should_vibrate_on_hour);
	read_legacy_pref(PERSIST_PREF_VIBRATE_ON_DISCONNECT_DEPRECATED, prefs.should_vibrate_on_disconnect);
	read_legacy_pref(PERSIST_PREF_VIBRATE_ON_CONNECT_DEPRECATED, prefs.should_vibrate_on_connect);

	read_legacy_pref(PERSIST_PREF_QUIET_DURING_SLEEP_DEPRECATED, prefs.should_quiet_during_sleep);

	read_legacy_pref(PERSIST_PREF_LEFT_COMPLICATION_DEPRECATED, prefs.left_complication_type);
	read_legacy_pref(PERSIST_PREF_BOTTOM_COMPLICATION_DEPRECATED, prefs.bottom_complication_type);
	read_legacy_pref(PERSIST_PREF_RIGHT_COMPLICATION_DEPRECATED, prefs.right_complication_type);

	read_legacy_pref(PERSIST_PREF_LEFT_COMPLICATION_OPT1_DEPRECATED, prefs.left_complication_opt1);
	read_legacy_pref(PERSIST_PREF_LEFT_COMPLICATION_OPT2_DEPRECATED, prefs.left_complication_opt2);
	read_legacy_pref(PERSIST_PREF_LEFT_COMPLICATION_OPT3_DEPRECATED, prefs.left_complication_opt3);
	read_legacy_pref(PERSIST_PREF_LEFT_COMPLICATION_OPT4_DEPRECATED, prefs.left_complication_opt4);

	read_legacy_pref(PERSIST_PREF_BOTTOM_COMPLICATION_OPT1_DEPRECATED, prefs.bottom_complication_opt1);
	read_legacy_pref(PERSIST_PREF_BOTTOM_COMPLICATION_OPT2_DEPRECATED, prefs.bottom_complication_opt2);
	read_legacy_pref(PERSIST_PREF_BOTTOM_COMPLICATION_OPT3_DEPRECATED, prefs.bottom_complication_opt3);
	read_legacy_pref(PERSIST_PREF_BOTTOM_COMPLICATION_OPT4_DEPRECATED, prefs.bottom_complication_opt4);

	read_legacy_pref(PERSIST_PREF_RIGHT_COMPLICATION_OPT1_DEPRECATED, prefs.right_complication_opt1);
	read_legacy_pref(PERSIST_PREF_RIGHT_COMPLICATION_OPT2_DEPRECATED, prefs.right_complication_opt2);
	read_legacy_pref(PERSIST_PREF_RIGHT_COMPLICATION_OPT3_DEPRECATED, prefs.right_complication_opt3);
	read_legacy_pref(PERSIST_PREF_RIGHT_COMPLICATION_OPT4_DEPRECATED, prefs.right_complication_opt4);

	read_legacy_pref(PERSIST_PREF_THEME_DEPRECATED, prefs.theme);
}

static void delete_legacy_preferences()
{
	for(unsigned int pref = PERSIST_WEATHER_TEMP_C_DEPRECATED;
	    pref <= PERSIST_PREF_QUIET_DURING_SLEEP_DEPRECATED; ++pref)
	{
		persist::remove(pref);
	}
}

void init_preferences()
{
	if(persist::exists(PERSIST_PREFS_STRUCT)) {
		load_preferences();
	}
	else {
		// Upgrade from legacy key-based preferences.  This should only need to happen once.
		upgrade_legacy_preferences();
		save_preferences();
		delete_legacy_preferences();
	}
}
