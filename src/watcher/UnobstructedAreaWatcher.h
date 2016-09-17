#ifndef UNOBSTRUCTEDAREAWATCHER_H
#define UNOBSTRUCTEDAREAWATCHER_H

extern "C" {
#include <pebble.h>
}

#include <vector>

#if PBL_API_EXISTS(unobstructed_area_service_subscribe)
#define UNOBSTRUCTED_AREA
#endif

class UnobstructedAreaCallback {
#ifdef UNOBSTRUCTED_AREA
	static std::vector<UnobstructedAreaCallback*> callbacks;

protected:
	UnobstructedAreaCallback();
	~UnobstructedAreaCallback();

	virtual void on_area_will_change(const GRect&) = 0;
	virtual void on_area_change(AnimationProgress) = 0;
	virtual void on_area_did_change() {}

private:
	static void will_change(GRect final_unobstructed_screen_area, void*);
	static void change(AnimationProgress progress, void*);
	static void did_change(void*);
#endif
};

#endif
