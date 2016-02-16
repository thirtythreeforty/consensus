#include "complication.h"

#include "common.h"
#include "themes.h"

static uint32_t battery_complication_angle(const BatteryChargeState *state)
{
	// According to Pebble, apps should not display a percentage while
	// charging, because they cannot accurately measure the battery.
	// See https://www.reddit.com/r/pebble/comments/3bodnk/full_charge_at_70/
	return state->is_charging ? 0 : state->charge_percent * TRIG_MAX_ANGLE / 100;
}

BatteryComplication::BatteryComplication(GRect frame)
	: HighlightComplication(frame)
{}

void BatteryComplication::update(GContext *ctx)
{
	HighlightComplication::update(ctx);

	// Draw the icon based on the charging state (icon is loaded/unloaded
	// in battery_complication_state_changed)
	icon.draw(ctx);
}

void BatteryComplication::state_changed(const BatteryChargeState *charge)
{
	set_angle(battery_complication_angle(charge));

	// Update the icon
	const uint32_t resource = charge->is_charging
		? RESOURCE_ID_BATTERY_CHARGING : RESOURCE_ID_BATTERY;
	icon.reset(resource, get_bounds());

	mark_dirty();
}

GColor BatteryComplication::highlight_color() const {
	return GColorYellow;
}

void BatteryComplication::configure(const std::array<unsigned int, 4>& config)
{
	icon.recolor();
	mark_dirty();
}
