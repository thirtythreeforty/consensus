#include "BatteryWatcher.h"

#include <algorithm>

decltype(BatteryCallback::callbacks) BatteryCallback::callbacks;

BatteryCallback::BatteryCallback()
{
	if(callbacks.empty()) {
		battery_state_service_subscribe(BatteryCallback::update_handler);
	}
	callbacks.emplace_back(this);
}

BatteryCallback::~BatteryCallback()
{
	std::remove(callbacks.begin(), callbacks.end(), this);
	if(callbacks.empty()) {
		battery_state_service_unsubscribe();
	}
}

void BatteryCallback::update_battery_now()
{
	const BatteryChargeState charge_state = battery_state_service_peek();
	update_handler(charge_state);
}

void BatteryCallback::update_handler(BatteryChargeState state)
{
	for(const auto& callback: callbacks) {
		callback->on_battery_change(state);
	}
}
