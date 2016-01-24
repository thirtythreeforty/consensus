#include "complication.h"

#include "common.h"
#include "constants.h"

using std::experimental::nullopt;

static time_t start_of_today()
{
	time_t now = time(nullptr);
	struct tm* ltime = localtime(&now);
	ltime->tm_hour = 0;
	ltime->tm_min = 0;
	ltime->tm_sec = 0;
	return mktime(ltime);
}

HealthComplication::HealthComplication(GRect frame)
	: Complication(frame)
	, animating(false)
{
	// Try to restore from persistent storage
	const time_t day_start = start_of_today();
	// persist_read_int() returns 0 if not set; since it is no longer 1970,
	// this is acceptable.
	if(persist_read_int(PERSIST_HEALTH_TIME_OF_AVERAGE) >= day_start) {
		uint32_t average = persist_read_int(PERSIST_HEALTH_AVERAGE_STEPS);
		steps.emplace(0, average);
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
	if(steps) {
		persist_write_int(PERSIST_HEALTH_TIME_OF_AVERAGE, time(nullptr));
		persist_write_int(PERSIST_HEALTH_AVERAGE_STEPS, steps->average);
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
	if(steps) {
		steps->today = health_service_sum_today(HealthMetricStepCount);

		// TODO only change if needed
		icon.emplace(steps->today > steps->average ? RESOURCE_ID_HEALTH_CHECK : RESOURCE_ID_HEALTH);

		const GSize icon_size = icon->get_bounds_size();
		const GRect bounds = this->get_bounds();
		icon_shift = {
			.x = static_cast<int16_t>(bounds.size.w / 2 - icon_size.w / 2),
			.y = static_cast<int16_t>(bounds.size.h / 2 - icon_size.h / 2)
		};

		// TODO animate if large distance
		mark_dirty();
	}
	else {
		icon.emplace(RESOURCE_ID_HEALTH_ERROR);
	}
}

void HealthComplication::update(GContext *ctx)
{
	auto steps = this->steps.value_or(Steps(0, 1));
	redraw_1(ctx, GColorGreen, TRIG_MAX_ANGLE * steps.today / steps.average);
	if(icon) {
		icon->draw(ctx, icon_shift);
	}
}

void HealthComplication::recalculate_average_steps()
{
	constexpr time_t seconds_in_day = 60 * 60 * 24;
	time_t today_start = start_of_today();

	// No matter what we do, we'll need a redraw
	mark_dirty();

	for(unsigned int days = 7; days > 0; --days) {
		const time_t ago = today_start - seconds_in_day * days;
		if(health_service_metric_accessible(HealthMetricStepCount, ago, today_start) ==
		   HealthServiceAccessibilityMaskAvailable) {
			steps.emplace(0, health_service_sum(HealthMetricStepCount, ago, today_start) / days);
			return;
		}
	}
	steps = nullopt;
}
