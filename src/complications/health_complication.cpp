#include "complication.h"

#include "common.h"
#include "constants.h"

HealthComplication::HealthComplication(GRect frame)
	: HighlightComplication(frame)
{}

void HealthComplication::configure(const config_bundle_t& config)
{
	HighlightComplication::configure(config);

#ifdef PBL_HEALTH
	const auto user_goal = std::get<0>(config);
	if(user_goal != 0) {
		step_goal = ManualGoal(user_goal);
	}
	else {
		recalculate_average_steps();
	}
	on_movement_update();
#else
	set_icon(RESOURCE_ID_HEALTH_ERROR);
#endif
}

GColor HealthComplication::highlight_color() const
{
	return theme().health_complication_color();
}

#ifdef PBL_HEALTH
Variant<void, int32_t> HealthComplication::today_steps;

void HealthComplication::on_tick(TimeUnits units_changed)
{
	if(units_changed & MINUTE_UNIT) {
		update_angle_and_icon();
	}
}

void HealthComplication::on_significant_update()
{
	if(step_goal.is<AutoGoal>()) {
		recalculate_average_steps();
	}
	on_movement_update();

	// Don't wait for the minute to roll around in this case
	update_angle_and_icon();
}

void HealthComplication::on_movement_update()
{
	// This function will be called twice when the program is launched:
	// once for the configure() call from the main window, and one from the
	// significant update that the Pebble OS sends out.  I don't know of
	// any way to avoid this though.
	invalidate_steps_today();
}

void HealthComplication::recalculate_average_steps()
{
	constexpr time_t seconds_in_day = 60 * 60 * 24;
	time_t today_start = time_start_of_today();

	// Pebble takes the average over the past 30 days, although there isn't an API for this
	// yet. See https://pebbledev.slack.com/archives/health-beta/p1454373385000080 (private channel)
	for(unsigned int days = 30; days > 0; --days) {
		const time_t ago = today_start - seconds_in_day * days;
		if(health_service_metric_accessible(HealthMetricStepCount, ago, today_start) ==
		   HealthServiceAccessibilityMaskAvailable) {
			step_goal = AutoGoal(health_service_sum(HealthMetricStepCount, ago, today_start) / days);
			return;
		}
	}
	step_goal.reset();
}

void HealthComplication::refresh_steps_today()
{
	if(today_steps.is<void>()) {
		today_steps = health_service_sum_today(HealthMetricStepCount);
	}
}

void HealthComplication::invalidate_steps_today()
{
	today_steps.reset();
}

void HealthComplication::update_angle_and_icon()
{
	if(!step_goal.is<void>()) {
		refresh_steps_today();

		auto& goal = step_goal.as<Goal>().goal;
		auto& steps = today_steps.as<int32_t>();

		set_angle(TRIG_MAX_ANGLE * steps / goal);
		set_icon(steps > goal ? RESOURCE_ID_HEALTH_CHECK : RESOURCE_ID_HEALTH);
	}
	else {
		set_angle(0);
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	};
}
#endif
