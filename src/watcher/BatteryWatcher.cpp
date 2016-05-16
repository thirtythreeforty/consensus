#include "BatteryWatcher.h"

#include <algorithm>

decltype(BatteryCallback::callbacks) BatteryCallback::callbacks;
unsigned int BatteryCallback::n = 0;

BatteryCallback::BatteryCallback()
{
	callbacks[n++] = this;
	if(n == 1) {
		battery_state_service_subscribe(BatteryCallback::update_handler);
	}
}

BatteryCallback::~BatteryCallback()
{
	n = std::remove(&callbacks[0], &callbacks[n], this) - &callbacks[0];
}

void BatteryCallback::update_battery_now()
{
	const BatteryChargeState charge_state = battery_state_service_peek();
	update_handler(charge_state);
}

void BatteryCallback::update_handler(BatteryChargeState state)
{
	for(unsigned int i = 0; i < n; ++i) {
		callbacks[i]->on_battery_change(state);
	}
}
