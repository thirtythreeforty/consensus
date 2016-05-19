#include <array>
#include <memory>

extern "C" {
#include <pebble.h>
}

#include "constants.h"
#include "main_window.h"
#include "preferences.h"
#include "themes.h"
#include "vibration.h"

std::unique_ptr<MainWindow> main_window;

static enum {
	WAS_CONNECTED_INIT = 0,
	WAS_CONNECTED_FALSE,
	WAS_CONNECTED_TRUE
} was_connected = WAS_CONNECTED_INIT;

void on_connection_change(bool connected)
{
	if(main_window) {
		main_window->on_connection_change(connected);
	}

	const bool became_disconnected = was_connected == WAS_CONNECTED_TRUE && !connected;
	const bool became_connected = was_connected == WAS_CONNECTED_FALSE && connected;

	if(became_disconnected &&
	   vibration_ok() &&
	   should_vibrate_on_disconnect()) {
		static const uint32_t vibe_pattern[] = {200, 250, 200, 250, 800};
		static const VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = NELEM(vibe_pattern)
		};

		vibes_enqueue_custom_pattern(vibe);
	}

	if(became_connected &&
	   vibration_ok() &&
	   should_vibrate_on_connect()) {
		static const uint32_t vibe_pattern[] = {150, 100, 150};
		static const VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = NELEM(vibe_pattern)
		};

		vibes_enqueue_custom_pattern(vibe);
	}

	was_connected = connected ? WAS_CONNECTED_TRUE : WAS_CONNECTED_FALSE;
}

void ignore_connection_change(bool connected)
{
}

static void on_preferences_in(DictionaryIterator *iterator)
{
	parse_preferences(iterator);
	set_theme();

	if(main_window) {
		main_window->configure();
	}
}

void on_appmessage_in(DictionaryIterator *iterator, void *context)
{
	WeatherData wdata = WeatherData::from_appmessage(iterator);
	if(wdata.valid) {
		// If this is a preferences message, don't erase saved weather
		wdata.to_persist();
	}

	if(Boulder::Tuple(dict_find(iterator, KEY_PREFS_DONT_EXIST)).valid()) {
		prefs_dont_exist();
	}
	on_preferences_in(iterator);
}

void on_appmessage_in_dropped(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped (reason %i)!", reason);
}

void on_tap(AccelAxisType axis, int32_t direction)
{
	if(main_window && main_window->should_power_compass()) {
		main_window->on_compass_power(true);

		compass_service_subscribe([](CompassHeadingData heading){
			if(main_window) {
				main_window->on_compass_update(heading);
			}
		});

		Boulder::AppTimer::create(30000, []{
			compass_service_unsubscribe();
			if(main_window) {
				main_window->on_compass_power(false);
			}
		});

	}
}

static void init(void)
{
	init_preferences();
	set_theme();

	main_window = std::make_unique<MainWindow>();
	Boulder::WindowStack::push(*main_window, true);

	static const ConnectionHandlers conn_handlers = {
		.pebble_app_connection_handler = on_connection_change,
		.pebblekit_connection_handler = ignore_connection_change,
	};
	was_connected = connection_service_peek_pebble_app_connection() ? WAS_CONNECTED_TRUE : WAS_CONNECTED_FALSE;
	connection_service_subscribe(conn_handlers);

	accel_tap_service_subscribe(on_tap);

	app_message_register_inbox_received(on_appmessage_in);
	app_message_register_inbox_dropped(on_appmessage_in_dropped);
	app_message_open(256, 64);
}

static void deinit(void)
{
	animation_unschedule_all();
	app_message_deregister_callbacks();
	accel_tap_service_unsubscribe();
	connection_service_unsubscribe();

	main_window.reset();
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
