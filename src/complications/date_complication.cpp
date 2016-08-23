#include "complication.h"

#include "common.h"

DateComplication::DateComplication(GRect frame)
	: IconTextComplication(frame)
	, TimeCallback(DAY_UNIT)
{
	update_time_now();
}

void DateComplication::configure(const config_bundle_t& config)
{
	IconTextComplication::configure(config);

	unit_type = static_cast<UnitType>(std::get<0>(config));
	update_time_now();
}

void DateComplication::on_tick(struct tm *time, TimeUnits)
{
	set_number_format(plain_number_format, time->tm_mday, unit(time));
}

const char* DateComplication::unit(struct tm *time)
{
	switch(unit_type) {
	case DayOfWk:
		return weekday(time->tm_wday);
	case Month:
		return month(time->tm_mon);
	default:
		return empty;
	}
}

const char* DateComplication::weekday(int wday)
{
	static const char* weekdays[] = {
		"SUN",
		"MON",
		"TUE",
		"WED",
		"THU",
		"FRI",
		"SAT",
	};
	if(wday >= 0 && wday < 7) {
		return weekdays[wday];
	}
	else {
		return empty;
	}
}

const char* DateComplication::month(int m)
{
	static const char* months[] = {
		"JAN",
		"FEB",
		"MAR",
		"APR",
		"MAY",
		"JUN",
		"JUL",
		"AUG",
		"SEP",
		"OCT",
		"NOV",
		"DEC",
	};
	if(m >= 0 && m < 12) {
		return months[m];
	}
	else {
		return empty;
	}
}
