#include "complication.h"

#include "animated.h"
#include "rotating_icon.h"

CompassComplication::CompassComplication(GRect frame)
	: IconTextComplication(frame)
{}

void CompassComplication::configure(const config_bundle_t&)
{}

void CompassComplication::update(GContext* ctx)
{
	IconTextComplication::update(ctx);
}

static RotatingIcon foo;

void CompassComplication::on_power(bool on)
{
	foo.set_angle(3);
	if(on) {

	}
	else {

	}
}

void CompassComplication::on_update(CompassHeadingData& heading)
{

}
