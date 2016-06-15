#include "TimeWatcher.h"

#include <algorithm>

std::vector<TimeCallback::cb_t> TimeCallback::callbacks;

TimeCallback::TimeCallback(TimeUnits units)
{
	callbacks.emplace_back(this, units);
	resubscribe();
}

TimeCallback::~TimeCallback()
{
	std::remove_if(callbacks.begin(), callbacks.end(),
	               [this](auto c){ return c.first == this; });
	resubscribe();
}

void TimeCallback::update_time_subscription(TimeUnits units)
{
	for(auto& callback: callbacks) {
		if(callback.first == this) {
			callback.second = units;
		}
	}
	resubscribe();
}

void TimeCallback::tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	for(const auto& callback: callbacks) {
		if(callback.second & units_changed) {
			callback.first->on_tick(tick_time, units_changed);
		}
	}
}

void TimeCallback::resubscribe()
{
	if(callbacks.empty()) {
		tick_timer_service_unsubscribe();
		return;
	}

	for(const auto& callback: callbacks) {
		if(callback.second & SECOND_UNIT) {
			tick_timer_service_subscribe(SECOND_UNIT, TimeCallback::tick_handler);
			return;
		}
	}

	tick_timer_service_subscribe(MINUTE_UNIT, TimeCallback::tick_handler);
}

void TimeCallback::update_time_now()
{
	time_t now = time(nullptr);
	struct tm *t = localtime(&now);
	static const TimeUnits all_changed = (TimeUnits)(
		YEAR_UNIT |
		MONTH_UNIT |
		DAY_UNIT |
		HOUR_UNIT |
		MINUTE_UNIT |
		SECOND_UNIT);
	on_tick(t, all_changed);
}
