#include "complication.h"

TimeZoneComplication::TimeZoneComplication(GRect frame)
	: Complication(frame)
	, TimeCallback(MINUTE_UNIT)
	, face(get_bounds(), false)
{
	add_child(face);
	update_time();
	animation_schedule(face.animate_in(false, true));
}

void TimeZoneComplication::configure(const config_bundle_t& config)
{
	Complication::configure(config);

	offset_sec = std::get<0>(config);
	if(std::get<1>(config)) {
		offset_sec = -offset_sec;
	}

	update_time();
}

void TimeZoneComplication::on_tick(struct tm*, TimeUnits)
{
	update_time();
}

void TimeZoneComplication::update_time()
{
	time_t utc = time(nullptr);
	utc += offset_sec;
	struct tm *offset_time = gmtime(&utc);

	face.set_time(offset_time->tm_hour, offset_time->tm_min, offset_time->tm_sec);
}
