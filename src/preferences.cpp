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

	bool should_animate = true;

	uint8_t top_complication_type = AbstractComplication::typenum_of<StatusComplication>();
	uint32_t top_complication_opt1 = 0;
	uint32_t top_complication_opt2 = 0;
	uint32_t top_complication_opt3 = 0;
	uint32_t top_complication_opt4 = 0;
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

bool should_animate()
{
	return prefs.should_animate;
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

complication_config top_complication_type()
{
	return {
		prefs.top_complication_type,
		{
			prefs.top_complication_opt1,
			prefs.top_complication_opt2,
			prefs.top_complication_opt3,
			prefs.top_complication_opt4,
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

	update_preference(iterator, KEY_PREF_SHOW_ANIMATIONS, prefs.should_animate);

	update_preference(iterator, KEY_PREF_TOP_COMPLICATION, prefs.top_complication_type);
	update_preference(iterator, KEY_PREF_TOP_COMPLICATION_OPT1, prefs.top_complication_opt1);
	update_preference(iterator, KEY_PREF_TOP_COMPLICATION_OPT2, prefs.top_complication_opt2);
	update_preference(iterator, KEY_PREF_TOP_COMPLICATION_OPT3, prefs.top_complication_opt3);
	update_preference(iterator, KEY_PREF_TOP_COMPLICATION_OPT4, prefs.top_complication_opt4);

	save_preferences();
}

void init_preferences()
{
	if(persist::exists(PERSIST_PREFS_STRUCT)) {
		load_preferences();
	}
	else {
		// Request prefs after JS has time to start
		Boulder::AppTimer::create(1000, []{
			DictionaryIterator *iter;
			static const uint8_t value = 0;
			APP_LOG(APP_LOG_LEVEL_INFO, "Requesting previous preferences");
			app_message_outbox_begin(&iter);
			dict_write_int(iter, KEY_PREFS_REQUEST, &value, sizeof value, false);
			app_message_outbox_send();
		});
	}
}

void prefs_dont_exist()
{
	// Just save the defaults so we don't bother the phone again next time
	APP_LOG(APP_LOG_LEVEL_INFO, "Previous preferences do not exist");
	save_preferences();
}
