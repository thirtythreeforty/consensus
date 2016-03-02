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

void HealthComplication::on_significant_update()
{
#ifdef PBL_HEALTH
	if(step_goal.is<AutoGoal>()) {
		recalculate_average_steps();
	}
	on_movement_update();
#endif
}

void HealthComplication::on_movement_update()
{
	// This function will be called twice when the program is launched:
	// once for the configure() call from the main window, and one from the
	// significant update that the Pebble OS sends out.  I don't know of
	// any way to avoid this though.
#ifdef PBL_HEALTH
	auto onValid = [&](Goal& goal){
		uint32_t today_steps = health_service_sum_today(HealthMetricStepCount);

		set_angle(TRIG_MAX_ANGLE * today_steps / goal.goal);
		set_icon(today_steps > goal.goal ? RESOURCE_ID_HEALTH_CHECK : RESOURCE_ID_HEALTH);
	};
	step_goal.if_is<AutoGoal>(onValid);
	step_goal.if_is<ManualGoal>(onValid);
	step_goal.if_is<void>([&]{
		set_angle(0);
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	});
#endif
}

bool HealthComplication::want_live_updates()
{
	// TODO with the center gadget change
	return false;
}

GColor HealthComplication::highlight_color() const
{
	return theme().health_complication_color();
}

void HealthComplication::recalculate_average_steps()
{
#ifdef PBL_HEALTH
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
#endif
}
