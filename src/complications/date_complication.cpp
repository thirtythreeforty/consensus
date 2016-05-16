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

	unit_type = static_cast<UnitType>(std::get<1>(config));
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
	switch(wday) {
	case 0:
		return "SUN";
	case 1:
		return "MON";
	case 2:
		return "TUE";
	case 3:
		return "WED";
	case 4:
		return "THU";
	case 5:
		return "FRI";
	case 6:
		return "SAT";
	default:
		return empty;
	}
}

const char* DateComplication::month(int m)
{
	switch(m) {
	case 0:
		return "JAN";
	case 1:
		return "FEB";
	case 2:
		return "MAR";
	case 3:
		return "APR";
	case 4:
		return "MAY";
	case 5:
		return "JUN";
	case 6:
		return "JUL";
	case 7:
		return "AUG";
	case 8:
		return "SEP";
	case 9:
		return "OCT";
	case 10:
		return "NOV";
	case 11:
		return "DEC";
	default:
		return empty;
	}
}
