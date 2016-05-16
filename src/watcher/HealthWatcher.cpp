#include "HealthWatcher.h"

#include <algorithm>

std::array<HealthCallback*, 4> HealthCallback::callbacks;
unsigned int HealthCallback::n;

HealthCallback::HealthCallback()
{
#ifdef PBL_HEALTH
	callbacks[n++] = this;
	if(n == 1) {
		health_service_events_subscribe(HealthCallback::update_handler, this);
	}
#endif
}

HealthCallback::~HealthCallback()
{
#ifdef PBL_HEALTH
	n = std::remove(&callbacks[0], &callbacks[n], this) - &callbacks[0];
	if(n == 0) {
		health_service_events_unsubscribe();
	}
#endif
}

#ifdef PBL_HEALTH
void HealthCallback::update_handler(HealthEventType ev, void* ptr)
{
	for(unsigned int i = 0; i < n; ++i) {
		if(ev == HealthEventSignificantUpdate) {
			callbacks[i]->on_significant_update();
		}
		else if(ev == HealthEventMovementUpdate) {
			callbacks[i]->on_movement_update();
		}
	}
}
#endif
