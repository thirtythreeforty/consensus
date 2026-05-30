#include "complication.h"

#include "common.h"
#include "constants.h"

#include <algorithm>

#ifdef PBL_HEALTH
namespace {
uint16_t configured_bpm_threshold(unsigned configured_value, uint16_t default_value)
{
	return configured_value
		? static_cast<uint16_t>(configured_value)
		: default_value;
}
}
#endif

HealthComplication::HealthComplication(GRect frame)
	: TickComplication(frame)
#ifdef PBL_HEALTH
	, TimeCallback(MINUTE_UNIT)
	, gadget_type(ICON)
	, heart_rate_zone2_min(default_heart_rate_zone2_min)
	, heart_rate_zone3_min(default_heart_rate_zone3_min)
#endif
{}

void HealthComplication::configure(const config_bundle_t& config)
{
	TickComplication::configure(config);

#ifdef PBL_HEALTH
	gadget_type = static_cast<GadgetType>(std::get<1>(config));
	configure_heart_rate_zones(config);

	const auto user_goal = std::get<0>(config);
	if(user_goal != 0) {
		step_goal = ManualGoal(user_goal);
	}
	else {
		recalculate_auto_goal();
	}

	update_gadget();
#else
	set_icon(RESOURCE_ID_HEALTH_ERROR);
#endif
}

GColor HealthComplication::highlight_color() const
{
#ifdef PBL_HEALTH
	if(is_heart_rate_gadget()) {
		return heart_rate_zone_color();
	}
#endif
	return theme().health_complication_color();
}

GColor HealthComplication::tick_color() const
{
#ifdef PBL_HEALTH
	if(is_heart_rate_gadget()) {
		return GColorClear;
	}
#endif
	return theme().health_complication_dot_color();
}

#ifdef PBL_HEALTH
const HealthComplication::divisor_list_t<3> HealthComplication::time_divisors = {
	std::make_tuple(1, 60, "SEC", plain_number_format),
	std::make_tuple(60, 60, "MIN", plain_number_format),
	std::make_tuple(60, -1, "HR", plain_number_format)
};

const HealthComplication::divisor_list_t<3> HealthComplication::imperial_distance_divisors = {
	std::make_tuple(1, 528, "FT", plain_number_format),
	std::make_tuple(528, 10, "MI", point_number_format),
	std::make_tuple(10, -1, "MI", plain_number_format)
};

const HealthComplication::divisor_list_t<3> HealthComplication::metric_distance_divisors = {
	std::make_tuple(1, 100, "M", plain_number_format),
	std::make_tuple(100, 10, "KM", point_number_format),
	std::make_tuple(10, -1, "KM", plain_number_format)
};

const HealthComplication::divisor_list_t<3> HealthComplication::calorie_divisors = {
	std::make_tuple(1, 100, "CAL", plain_number_format),
	std::make_tuple(100, 10, "KCAL", point_number_format),
	std::make_tuple(10, -1, "KCAL", plain_number_format),
};

Variant<void, int32_t> HealthComplication::st_today_steps;
Variant<void, int32_t> HealthComplication::st_today_average_steps;

void HealthComplication::on_tick(struct tm*, TimeUnits units_changed)
{
	update_gadget();
}

void HealthComplication::on_significant_update()
{
	if(step_goal.is<AutoGoal>()) {
		recalculate_auto_goal();
	}
	on_movement_update();
}

void HealthComplication::on_movement_update()
{
	invalidate_today_data();
	update_gadget();
}

void HealthComplication::on_heart_update()
{
	update_gadget();
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

void HealthComplication::store_scaled_metric(uint32_t metric, const divisor_t* begin, const divisor_t* end)
{
	auto i = begin;
	do {
		metric /= std::get<0>(*i);
		if(metric < std::get<1>(*i)) {
			++i;
			break;
		}
	} while(++i < end);

	--i;
	today_metric = Metric(metric, std::get<2>(*i), std::get<3>(*i));
}

bool HealthComplication::is_heart_rate_gadget() const
{
	return gadget_type == HEART_RATE_ICON || gadget_type == HEART_RATE_BPM;
}

void HealthComplication::configure_heart_rate_zones(const config_bundle_t& config)
{
	heart_rate_zone2_min = configured_bpm_threshold(
		std::get<2>(config),
		default_heart_rate_zone2_min);
	heart_rate_zone3_min = std::max<uint16_t>(
		configured_bpm_threshold(std::get<3>(config), default_heart_rate_zone3_min),
		heart_rate_zone2_min + 1);
}

void HealthComplication::update_gadget()
{
	if(is_heart_rate_gadget()) {
		update_heart_rate_gadget();
	}
	else {
		update_step_gadget();
	}
}

void HealthComplication::update_heart_rate_gadget()
{
	const HealthValue heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);

	if(heart_rate > 0) {
		current_heart_rate = heart_rate;
		const HealthValue ring_heart_rate = std::min(heart_rate, (HealthValue)heart_rate_ring_max);
		angle = TRIG_MAX_ANGLE * ring_heart_rate / heart_rate_ring_max;

		if(gadget_type == HEART_RATE_BPM) {
			set_number_format(plain_number_format, heart_rate, "BPM");
		}
		else {
			set_icon(RESOURCE_ID_HEALTH);
		}
	}
	else {
		current_heart_rate.reset();
		angle = 0;
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	}
}

GColor HealthComplication::heart_rate_zone_color() const
{
#ifndef PBL_COLOR
	return theme().health_complication_color();
#else
	if(current_heart_rate.is<void>()) {
		return theme().health_complication_color();
	}

	const auto heart_rate = current_heart_rate.as<HealthValue>();
	if(heart_rate < heart_rate_zone2_min) {
		return GColorGreen;
	}
	else if(heart_rate < heart_rate_zone3_min) {
		return GColorYellow;
	}
	else {
		return GColorRed;
	}
#endif
}

void HealthComplication::recalculate_today_data()
{
	refresh_static_steps_today();

	if(today_metric.is<void>()) {
		switch(gadget_type) {
		case ICON:
			break;
		case STEPS:
			if(st_today_steps.is<void>()) {
				today_metric.reset();
			}
			else {
				today_metric = Metric(st_today_steps.as<int32_t>(), "STEPS", plain_number_format);
			}
			break;
		case DISTANCE:
			if(health_service_get_measurement_system_for_display(HealthMetricWalkedDistanceMeters) == MeasurementSystemMetric) {
				store_scaled_metric(
					health_service_sum_today(HealthMetricWalkedDistanceMeters),
					metric_distance_divisors);
			}
			else {
				store_scaled_metric(
					health_service_sum_today(HealthMetricWalkedDistanceMeters) * 328 / 100,
					imperial_distance_divisors);
			}
			break;
		case CALORIES_ACTIVE:
			store_scaled_metric(
				health_service_sum_today(HealthMetricActiveKCalories),
				calorie_divisors);
			break;
		case CALORIES_RESTING:
			store_scaled_metric(
				health_service_sum_today(HealthMetricRestingKCalories),
				calorie_divisors);
			break;
		case CALORIES_TOTAL:
			store_scaled_metric(
				health_service_sum_today(HealthMetricActiveKCalories) + health_service_sum_today(HealthMetricRestingKCalories),
				calorie_divisors);
			break;
		case ACTIVE_SECONDS:
			store_scaled_metric(
				health_service_sum_today(HealthMetricActiveSeconds),
				time_divisors);
			break;
		case HEART_RATE_ICON:
		case HEART_RATE_BPM:
			break;
		}
	}
}

void HealthComplication::update_step_gadget()
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
			auto& tm = today_metric.as<Metric>();
			set_number_format(tm.format, tm.metric, tm.unit);
			break;
		}
	}
	else {
		angle = 0;
		set_icon(RESOURCE_ID_HEALTH_ERROR);
	};
}
#endif
