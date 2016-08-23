#ifndef DIGITAL_CLOCK_H
#define DIGITAL_CLOCK_H

#include "boulder.h"
#include "watcher/TimeWatcher.h"

class DigitalClock final: private TimeCallback
{
	Boulder::TextLayer text_layer;

public:
	explicit DigitalClock(GRect frame);

	void configure();

	void set_time(uint8_t hour, uint8_t min, uint8_t sec);

private:
	virtual void on_tick(struct tm*, TimeUnits) override;
};

#endif
