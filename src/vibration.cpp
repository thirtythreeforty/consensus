#include "vibration.h"

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

bool vibration_ok()
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
