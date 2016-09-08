#ifndef VIBRATION_H
#define VIBRATION_H

extern "C" {
#include "pebble.h"
}

#include "watcher/ConnectionWatcher.h"
#include "watcher/TimeWatcher.h"

class Vibrator: private ConnectionCallback, private TimeCallback {
	bool was_connected;

public:
	Vibrator();

	void on_connection_change(bool connected) override;
	void on_tick(struct tm *tick_time, TimeUnits units_changed) override;
};

#endif
