#include "complication.h"

#include "common.h"
#include "constants.h"

using std::experimental::nullopt;

HealthComplication::HealthComplication(GRect frame)
	: HighlightComplication(frame)
{}

void HealthComplication::configure(const config_bundle_t& config)
{
	HighlightComplication::configure(config);

	const auto& user_goal = std::get<0>(config);
	if(user_goal != 0) {
		step_goal = (uint32_t)user_goal;
	}
	else {
		recalculate_average_steps();
	}
	on_movement_update();
}

void HealthComplication::on_significant_update()
{
	recalculate_average_steps();
	on_movement_update();
}

void HealthComplication::on_movement_update()
{
	step_goal.if_is_else<uint32_t>([&](uint32_t& goal){
		uint32_t today_steps = health_service_sum_today(HealthMetricStepCount);

		set_angle(TRIG_MAX_ANGLE * today_steps / goal);
		set_icon(today_steps > goal ? RESOURCE_ID_HEALTH_CHECK : RESOURCE_ID_HEALTH);
	}, [&]{
		set_angle(0);
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	});

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
			step_goal = health_service_sum(HealthMetricStepCount, ago, today_start) / days;
			return;
		}
	}
	step_goal.reset();
}
