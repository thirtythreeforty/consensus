#ifndef CONSTANTS_H
#define CONSTANTS_H

// AppMessage keys:
enum {
	KEY_WEATHER_REQUEST = 0,
	KEY_WEATHER_TEMP_C,
	KEY_WEATHER_RAINCHANCE_PCT,
	KEY_WEATHER_CONDITIONS,
	KEY_WEATHER_HUMIDITY,
	KEY_WEATHER_ICON,

	KEY_PREF_SHOW_SECOND_HAND,
	KEY_PREF_SHOW_NO_CONNECTION,

	KEY_PREF_VIBRATE_ON_HOUR,
	KEY_PREF_VIBRATE_ON_DISCONNECT,

	KEY_PREF_LEFT_COMPLICATION,
	KEY_PREF_BOTTOM_COMPLICATION,
	KEY_PREF_RIGHT_COMPLICATION,
};

// Persistent storage keys:
enum {
	PERSIST_WEATHER_TEMP_C = 0,
	PERSIST_WEATHER_RAINCHANCE_PCT,
	PERSIST_WEATHER_CONDITIONS,
	PERSIST_WEATHER_HUMIDITY,
	PERSIST_WEATHER_ICON,
	PERSIST_WEATHER_TIME_UPDATED,

	PERSIST_PREF_SHOW_SECOND_HAND,
	PERSIST_PREF_SHOW_NO_CONNECTION,

	PERSIST_PREF_VIBRATE_ON_HOUR,
	PERSIST_PREF_VIBRATE_ON_DISCONNECT,

	PERSIST_PREF_LEFT_COMPLICATION,
	PERSIST_PREF_BOTTOM_COMPLICATION,
	PERSIST_PREF_RIGHT_COMPLICATION,
};

#endif
