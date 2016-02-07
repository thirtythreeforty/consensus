#include "complication.h"

#include "common.h"

DateComplication::DateComplication(GRect frame)
	: Complication(frame)
	, date(get_bounds(), "%02i")
{
	add_child(date.get_text_layer());
}

void DateComplication::time_changed(struct tm *time)
{
	date.set(time->tm_mday);
}
