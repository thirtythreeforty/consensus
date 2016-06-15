#ifndef HEALTHWATCHER_H
#define HEALTHWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <vector>

class HealthCallback {
	static std::vector<HealthCallback*> callbacks;

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
