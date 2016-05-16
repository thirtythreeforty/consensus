#ifndef BATTERYWATCHER_H
#define BATTERYWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <array>

class BatteryCallback {
	static std::array<BatteryCallback*, 4> callbacks;
	static unsigned int n;

protected:
	BatteryCallback();
	~BatteryCallback();

	virtual void on_battery_change(const BatteryChargeState&) = 0;

	void update_battery_now();

private:
	static void update_handler(BatteryChargeState state);
};

#endif
