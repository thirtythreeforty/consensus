#include "complication.h"

#include "common.h"

using std::experimental::nullopt;

HealthComplication::HealthComplication(GRect frame)
	: Complication(frame)
	, animating(false)
	, steps(nullopt)
{
	on_significant_update();
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
		APP_DEBUG("discovered %lu steps", steps->today);

		// TODO only change if needed
		// TODO icon for no health data
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
	time_t now = time(nullptr);

	for(unsigned int days = 7; days > 0; --days) {
		const time_t ago = now - seconds_in_day * days;
		if(health_service_metric_accessible(HealthMetricStepCount, ago, now) ==
		   HealthServiceAccessibilityMaskAvailable) {
			steps.emplace(0, health_service_sum(HealthMetricStepCount, ago, now) / days);

			APP_DEBUG("%u days valid, total is %lu, average is %lu!", days, health_service_sum(HealthMetricStepCount, ago, now), steps->average);

			mark_dirty();
			return;
		}
		APP_DEBUG("%i days not valid", days);
	}

	steps = nullopt;
	mark_dirty();
}
