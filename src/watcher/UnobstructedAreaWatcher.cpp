#include "UnobstructedAreaWatcher.h"

#ifdef UNOBSTRUCTED_AREA

#include <algorithm>

decltype(UnobstructedAreaCallback::callbacks) UnobstructedAreaCallback::callbacks;

UnobstructedAreaCallback::UnobstructedAreaCallback()
{
	if(callbacks.empty()) {
		static const UnobstructedAreaHandlers handlers = (UnobstructedAreaHandlers) {
			.will_change = UnobstructedAreaCallback::will_change,
			.change = UnobstructedAreaCallback::change,
			.did_change = UnobstructedAreaCallback::did_change
		};
		unobstructed_area_service_subscribe(handlers, nullptr);
	}
	callbacks.emplace_back(this);
}

UnobstructedAreaCallback::~UnobstructedAreaCallback()
{
	std::remove(callbacks.begin(), callbacks.end(), this);
	if(callbacks.empty()) {
		unobstructed_area_service_unsubscribe();
	}
}

void UnobstructedAreaCallback::will_change(GRect final_unobstructed_screen_area, void*)
{
	for(const auto& callback: callbacks) {
		callback->on_area_will_change(final_unobstructed_screen_area);
	}
}

void UnobstructedAreaCallback::change(AnimationProgress progress, void*)
{
	for(const auto& callback: callbacks) {
		callback->on_area_change(progress);
	}
}

void UnobstructedAreaCallback::did_change(void*)
{
	for(const auto& callback: callbacks) {
		callback->on_area_did_change();
	}
}

#endif
