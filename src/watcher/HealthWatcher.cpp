#include "HealthWatcher.h"

#include <algorithm>

std::vector<HealthCallback*> HealthCallback::callbacks;

HealthCallback::HealthCallback()
{
#ifdef PBL_HEALTH
	if(callbacks.empty()) {
		health_service_events_subscribe(HealthCallback::update_handler, this);
	}
	callbacks.emplace_back(this);
#endif
}

HealthCallback::~HealthCallback()
{
#ifdef PBL_HEALTH
	std::remove(callbacks.begin(), callbacks.end(), this);
	if(callbacks.empty()) {
		health_service_events_unsubscribe();
	}
#endif
}

#ifdef PBL_HEALTH
void HealthCallback::update_handler(HealthEventType ev, void* ptr)
{
	for(const auto& callback: callbacks) {
		if(ev == HealthEventSignificantUpdate) {
			callback->on_significant_update();
		}
		else if(ev == HealthEventMovementUpdate) {
			callback->on_movement_update();
		}
	}
}
#endif
