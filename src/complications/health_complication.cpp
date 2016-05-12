#include "complication.h"

#include "common.h"
#include "constants.h"

HealthComplication::HealthComplication(GRect frame)
	: TickComplication(frame)
{}

void HealthComplication::configure(const config_bundle_t& config)
{
	TickComplication::configure(config);

#ifdef PBL_HEALTH
	gadget_type = static_cast<GadgetType>(std::get<1>(config));
	const auto user_goal = std::get<0>(config);
	if(user_goal != 0) {
		step_goal = ManualGoal(user_goal);
	}
	else {
		recalculate_auto_goal();
	}
	on_movement_update();
	update_angle_and_gadget();
#else
	set_icon(RESOURCE_ID_HEALTH_ERROR);
#endif
}

GColor HealthComplication::highlight_color() const
{
	return theme().health_complication_color();
}

GColor HealthComplication::tick_color() const
{
	return theme().health_complication_dot_color();
}

#ifdef PBL_HEALTH
Variant<void, int32_t> HealthComplication::st_today_steps;
Variant<void, int32_t> HealthComplication::st_today_average_steps;

void HealthComplication::on_tick(TimeUnits units_changed)
{
	if(units_changed & MINUTE_UNIT) {
		update_angle_and_gadget();
	}
}

void HealthComplication::on_significant_update()
{
	if(step_goal.is<AutoGoal>()) {
		recalculate_auto_goal();
	}
	on_movement_update();

	// Don't wait for the minute to roll around in this case
	update_angle_and_gadget();
}

void HealthComplication::on_movement_update()
{
	// This function will be called twice when the program is launched:
	// once for the configure() call from the main window, and one from the
	// significant update that the Pebble OS sends out.  I don't know of
	// any way to avoid this though.
	invalidate_today_data();
}

void HealthComplication::recalculate_auto_goal()
{
	constexpr time_t seconds_in_day = 60 * 60 * 24;
	time_t today_start = time_start_of_today();

	if(health_service_metric_averaged_accessible(
			HealthMetricStepCount,
			today_start,
			today_start + seconds_in_day,
			HealthServiceTimeScopeDailyWeekdayOrWeekend))
	{
		step_goal = AutoGoal(
			health_service_sum_averaged(
					HealthMetricStepCount,
					today_start,
					today_start + seconds_in_day,
					HealthServiceTimeScopeDailyWeekdayOrWeekend));
	}
	else {
		step_goal.reset();
	}
}

void HealthComplication::refresh_static_steps_today()
{
	if(st_today_steps.is<void>()) {
		st_today_steps = health_service_sum_today(HealthMetricStepCount);
	}
	if(st_today_average_steps.is<void>()) {
		st_today_average_steps = health_service_sum_averaged(
			HealthMetricStepCount,
			time_start_of_today(),
			time(nullptr),
			HealthServiceTimeScopeDailyWeekdayOrWeekend
		);
	}
}

void HealthComplication::invalidate_static_today_data()
{
	st_today_steps.reset();
	st_today_average_steps.reset();
}

void HealthComplication::invalidate_today_data()
{
	invalidate_static_today_data();
	today_metric.reset();
}

void HealthComplication::recalculate_today_data()
{
	refresh_static_steps_today();

	if(today_metric.is<void>()) {
		switch(gadget_type) {
		case ICON:
			break;
		case STEPS:
			today_metric = st_today_steps;
			break;
		case DISTANCE_M:
			today_metric = health_service_sum_today(HealthMetricWalkedDistanceMeters);
			break;
		case DISTANCE_FT:
			today_metric = health_service_sum_today(HealthMetricWalkedDistanceMeters) * 328 / 100;
			break;
		case CALORIES_ACTIVE:
			today_metric = health_service_sum_today(HealthMetricActiveKCalories);
			break;
		case CALORIES_RESTING:
			today_metric = health_service_sum_today(HealthMetricRestingKCalories);
			break;
		case CALORIES_TOTAL:
			today_metric = health_service_sum_today(HealthMetricRestingKCalories)
			             + health_service_sum_today(HealthMetricActiveKCalories);
			break;
		case ACTIVE_SECONDS:
			today_metric = health_service_sum_today(HealthMetricActiveSeconds);
			break;
		}
	}
}

void HealthComplication::update_angle_and_gadget()
{
	if(!step_goal.is<void>()) {
		recalculate_today_data();

		auto& goal = step_goal.as<Goal>().goal;
		auto& steps = st_today_steps.as<int32_t>();
		auto& avg_steps = st_today_average_steps.as<int32_t>();

		angle = TRIG_MAX_ANGLE * steps / goal;
		tick_angle = TRIG_MAX_ANGLE * avg_steps / goal;

		switch(gadget_type) {
		case ICON:
			set_icon(steps > goal ? RESOURCE_ID_HEALTH_CHECK : RESOURCE_ID_HEALTH);
			break;
		default:
			set_number_format(plain_number_format, today_metric.as<int32_t>(), "steps");
			break;
		}
	}
	else {
		angle = 0;
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	};
}
#endif
