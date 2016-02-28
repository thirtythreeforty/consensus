#include <array>
#include <memory>

extern "C" {
#include <pebble.h>
}

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

#ifdef PBL_HEALTH
static void on_health_update(HealthEventType event, void*)
{
	if(main_window) {
		main_window->on_health_update(event);
	}
}
#endif

void on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	if(main_window) {
		main_window->on_tick(tick_time, units_changed);
	}
}

void on_battery_state_change(BatteryChargeState charge)
{
	if(main_window) {
		main_window->on_battery_state_change(charge);
	}
}

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

static void update_time_now()
{
	time_t abs_time = time(NULL);
	struct tm *tick_time = localtime(&abs_time);
	on_tick(tick_time, MINUTE_UNIT);
}

static TimeUnits update_time_interval(bool show_sec)
{
	return show_sec ? SECOND_UNIT : MINUTE_UNIT;
}

static void on_preferences_in(DictionaryIterator *iterator)
{
	parse_preferences(iterator);
	set_theme();

	const bool show_second = should_show_second();
	const TimeUnits units = update_time_interval(show_second);
	tick_timer_service_subscribe(units, on_tick);
	update_time_now();

	if(main_window) {
		main_window->configure();
	}
}

void on_appmessage_in(DictionaryIterator *iterator, void *context)
{
	WeatherData wdata;
	weather_from_appmessage(iterator, &wdata);
	// Weather complications are getting recreated below, so no point in telling them about this now
	if(wdata.valid) {
		weather_to_persist(&wdata);
	}

	on_preferences_in(iterator);
}

void on_appmessage_in_dropped(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped (reason %i)!", reason);
}

static void init(void)
{
	set_theme();

	main_window = std::make_unique<MainWindow>();
	main_window->push(true);

	const TimeUnits units = update_time_interval(should_show_second());
	tick_timer_service_subscribe(units, on_tick);

	battery_state_service_subscribe(on_battery_state_change);

	static const ConnectionHandlers conn_handlers = {
		.pebble_app_connection_handler = on_connection_change,
		.pebblekit_connection_handler = ignore_connection_change,
	};
	connection_service_subscribe(conn_handlers);

#ifdef PBL_HEALTH
	health_service_events_subscribe(on_health_update, nullptr);
#endif

	app_message_register_inbox_received(on_appmessage_in);
	app_message_register_inbox_dropped(on_appmessage_in_dropped);
	app_message_open(128, 64);

	update_time_now();
}

static void deinit(void)
{
	animation_unschedule_all();
	app_message_deregister_callbacks();
#ifdef PBL_HEALTH
	health_service_events_unsubscribe();
#endif
	connection_service_unsubscribe();
	battery_state_service_unsubscribe();
	tick_timer_service_unsubscribe();

	main_window.reset();
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
