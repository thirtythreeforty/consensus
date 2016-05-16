#include "TimeWatcher.h"

#include <algorithm>

std::array<TimeCallback::cb_t, 4> TimeCallback::callbacks;
unsigned int TimeCallback::n = 0;

TimeCallback::TimeCallback(TimeUnits units)
{
	callbacks[n++] = {this, units};
	resubscribe();
}

TimeCallback::~TimeCallback()
{
	n = std::remove_if(&callbacks[0], &callbacks[n],
	                    [this](auto c){ return c.first == this; })
	    - &callbacks[0];
	resubscribe();
}

void TimeCallback::update_time_subscription(TimeUnits units)
{
	for(unsigned int i = 0; i < n; ++i) {
		if(callbacks[i].first == this) {
			callbacks[i].second = units;
		}
	}
	resubscribe();
}

void TimeCallback::tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	for(unsigned int i = 0; i < n; ++i) {
		if(callbacks[i].second & units_changed) {
			callbacks[i].first->on_tick(tick_time, units_changed);
		}
	}
}

void TimeCallback::resubscribe()
{
	if(n == 0) {
		tick_timer_service_unsubscribe();
		return;
	}

	for(unsigned int i = 0; i < n; ++i) {
		if(callbacks[i].second & SECOND_UNIT) {
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
