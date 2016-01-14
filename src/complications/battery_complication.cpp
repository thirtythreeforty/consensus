#include "complication_common.h"

#include "common.h"

static uint32_t battery_complication_angle(const BatteryChargeState *state)
{
	// According to Pebble, apps should not display a percentage while
	// charging, because they cannot accurately measure the battery.
	// See https://www.reddit.com/r/pebble/comments/3bodnk/full_charge_at_70/
	return state->is_charging ? 0 : state->charge_percent * TRIG_MAX_ANGLE / 100;
}

void BatteryComplication::update(GContext *ctx)
{
	base_complication_update_1(*this, ctx, GColorYellow, angle);

	// Draw the icon based on the charging state (icon is loaded/unloaded
	// in battery_complication_state_changed)
	if(icon) {
		icon->draw(ctx, GPointZero);
	}
}

BatteryComplication::BatteryComplication(GRect frame)
	: Complication(frame)
	, animating(false)
	, angle(0)
{}

void BatteryComplication::state_changed(const BatteryChargeState *charge)
{
	requested_state = *charge;

	if(!animating) {
		animate_to(battery_complication_angle(charge));
	}

	// Update the icon
	const uint32_t resource = charge->is_charging
		? RESOURCE_ID_BATTERY_CHARGING : RESOURCE_ID_BATTERY;
	icon.emplace(resource);

	mark_dirty();
}

void BatteryComplication::spinup_started(Animation *anim, void *context)
{
	auto *complication = static_cast<BatteryComplication*>(context);
	complication->animating = true;
}

void BatteryComplication::spinup_stopped(Animation *anim, bool finished, void *context)
{
	auto *complication = static_cast<BatteryComplication*>(context);

	complication->animating = false;

	// The only way this function will be called is if the animation has
	// finished, which can only be the case if
	// battery_complication_state_changed was called, so data->requested_state
	// will have been initialized.
	const uint32_t requested_angle = battery_complication_angle(&complication->requested_state);

	if(requested_angle != complication->angle) {
		// Re-animate to the correct angle.
		complication->animate_to(requested_angle);
	}
}

uint32_t BatteryComplication::angle_get(void *subject)
{
	auto *complication = static_cast<BatteryComplication*>(subject);

	return complication->angle;
}

void BatteryComplication::angle_set(void *subject, uint32_t angle)
{
	auto *complication = static_cast<BatteryComplication*>(subject);

	complication->angle = angle;
	complication->mark_dirty();
}

void BatteryComplication::animate_to(uint32_t requested_angle)
{
	static const PropertyAnimationImplementation battery_spin_impl = {
		{
			NULL,
			(AnimationUpdateImplementation)property_animation_update_uint32,
			NULL
		},
		{
			reinterpret_cast<Int16Setter>(BatteryComplication::angle_set),
			reinterpret_cast<Int16Getter>(BatteryComplication::angle_get),
		}
	};
	static const AnimationHandlers battery_spinup_anim_handlers = {
		BatteryComplication::spinup_started,
		BatteryComplication::spinup_stopped
	};

	uint32_t current_angle = angle;
	PropertyAnimation *property_anim = property_animation_create(
		&battery_spin_impl,
		this,
		NULL, NULL
	);
	property_animation_from(property_anim, &current_angle, sizeof(current_angle), true);
	property_animation_to(property_anim, &requested_angle, sizeof(requested_angle), true);
	Animation *anim = property_animation_get_animation(property_anim);
	animation_schedule(base_complication_setup_animation(anim, &battery_spinup_anim_handlers, this));
}

