#include "vibration.h"

#include "common.h"
#include "preferences.h"

static bool user_is_asleep()
{
#ifdef PBL_HEALTH
	if(health_service_metric_accessible(HealthMetricSleepSeconds, time_start_of_today(), time(nullptr))) {
		return (health_service_peek_current_activities()
		        & (HealthActivitySleep | HealthActivityRestfulSleep));
	}
	else {
		return false;
	}
#else
	return false;
#endif
}

static bool vibration_ok()
{
	// Vibration is allowed if the user is not asleep or the user doesn't care
	// if we vibrate while they are asleep
	if(user_is_asleep()) {
		return !should_quiet_during_sleep();
	}
	else {
		return true;
	}
}

Vibrator::Vibrator()
	: TimeCallback(HOUR_UNIT)
	, was_connected(connection_service_peek_pebble_app_connection())
{}

void Vibrator::on_connection_change(bool connected)
{
	const bool became_disconnected = was_connected && !connected;
	const bool became_connected = !was_connected && connected;

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

	was_connected = connected;
}

void Vibrator::on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	// Vibrate once on the hour and twice at noon.
	if(tick_time->tm_min == 0 && tick_time->tm_sec == 0 &&
	   should_vibrate_on_hour() &&
	   vibration_ok()) {
		static const uint32_t vibe_pattern[] = {100, 250, 100};
		VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = static_cast<unsigned int>(tick_time->tm_hour % 12 == 0 ? 3 : 1)
		};

		vibes_enqueue_custom_pattern(vibe);
	}
}
