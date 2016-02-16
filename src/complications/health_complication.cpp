#include "complication.h"

#include "common.h"
#include "constants.h"

using std::experimental::nullopt;

HealthComplication::HealthComplication(GRect frame)
	: HighlightComplication(frame)
{
	// Try to restore from persistent storage
	const time_t day_start = time_start_of_today();
	// persist_read_int() returns 0 if not set; since it is no longer 1970,
	// this is acceptable.
	if(persist_read_int(PERSIST_HEALTH_TIME_OF_AVERAGE) >= day_start) {
		average_steps.emplace(persist_read_int(PERSIST_HEALTH_AVERAGE_STEPS));
		// Rest of the setup logic is identical to the update logic
		on_movement_update();
	}
	else {
		// Just recalculate...
		// Don't initialize steps, recalculate_average_steps() will handle that
		on_significant_update();
	}
}

HealthComplication::~HealthComplication()
{
	// If measurements are valid, persist them
	if(average_steps) {
		persist_write_int(PERSIST_HEALTH_TIME_OF_AVERAGE, time(nullptr));
		persist_write_int(PERSIST_HEALTH_AVERAGE_STEPS, *average_steps);
	}
	else {
		persist_delete(PERSIST_HEALTH_AVERAGE_STEPS);
		persist_delete(PERSIST_HEALTH_TIME_OF_AVERAGE);
	}
}

void HealthComplication::on_significant_update()
{
	recalculate_average_steps();
	on_movement_update();
}

void HealthComplication::on_movement_update()
{
	if(average_steps) {
		uint32_t today_steps = health_service_sum_today(HealthMetricStepCount);

		set_angle(TRIG_MAX_ANGLE * today_steps / *average_steps);
		set_icon(today_steps > *average_steps ? RESOURCE_ID_HEALTH_CHECK : RESOURCE_ID_HEALTH);
	}
	else {
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	}

	mark_dirty();
}

GColor HealthComplication::highlight_color() const
{
	return GColorGreen;
}

void HealthComplication::recalculate_average_steps()
{
	constexpr time_t seconds_in_day = 60 * 60 * 24;
	time_t today_start = time_start_of_today();

	// No matter what we do, we'll need a redraw
	mark_dirty();

	// Pebble takes the average over the past 30 days, although there isn't an API for this
	// yet. See https://pebbledev.slack.com/archives/health-beta/p1454373385000080 (private channel)
	for(unsigned int days = 30; days > 0; --days) {
		const time_t ago = today_start - seconds_in_day * days;
		if(health_service_metric_accessible(HealthMetricStepCount, ago, today_start) ==
		   HealthServiceAccessibilityMaskAvailable) {
			average_steps.emplace(health_service_sum(HealthMetricStepCount, ago, today_start) / days);
			return;
		}
	}
	average_steps = nullopt;
}
