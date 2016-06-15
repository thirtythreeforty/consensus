#ifndef TIMEWATCHER_H
#define TIMEWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <utility>
#include <vector>

class TimeCallback {
	using cb_t = std::pair<TimeCallback*, TimeUnits>;
	static std::vector<cb_t> callbacks;

protected:
	TimeCallback(TimeUnits units);
	~TimeCallback();
	void update_time_subscription(TimeUnits units);
	void update_time_now();

	virtual void on_tick(struct tm *, TimeUnits) = 0;

private:
	static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
	void resubscribe();
};

#endif
