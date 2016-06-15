#ifndef BATTERYWATCHER_H
#define BATTERYWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <vector>

class BatteryCallback {
	static std::vector<BatteryCallback*> callbacks;

protected:
	BatteryCallback();
	~BatteryCallback();

	virtual void on_battery_change(const BatteryChargeState&) = 0;

	void update_battery_now();

private:
	static void update_handler(BatteryChargeState state);
};

#endif
