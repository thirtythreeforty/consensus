#ifndef HEALTHWATCHER_H
#define HEALTHWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <algorithm>
#include <array>
#include <vector>

class HealthCallback {
	static std::array<HealthCallback*, 4> callbacks;
	static unsigned int n;

protected:
	HealthCallback();
	~HealthCallback();

#ifdef PBL_HEALTH
	virtual void on_movement_update() = 0;
	virtual void on_significant_update() = 0;
#endif

private:
	static void update_handler(HealthEventType ev, void* ptr);
};

#endif
