#include "complication_common.h"

#include "common.h"

static void battery_complication_animate_to(BatteryComplication *complication, uint32_t requested_angle);

typedef struct {
	bool animating;

	BatteryChargeState requested_state;

	uint32_t angle;
	GDrawCommandImage *icon;
} BatteryComplicationData;

static uint32_t battery_complication_angle(const BatteryChargeState *state)
{
	// According to Pebble, apps should not display a percentage while
	// charging, because they cannot accurately measure the battery.
	// See https://www.reddit.com/r/pebble/comments/3bodnk/full_charge_at_70/
	return state->is_charging ? 0 : state->charge_percent * TRIG_MAX_ANGLE / 100;
}

void battery_complication_update(Layer *layer, GContext *ctx)
{
	BatteryComplicationData *data = layer_get_data(layer);

	base_complication_update_1(layer, ctx, GColorYellow, data->angle);

	// Draw the icon based on the charging state (icon is loaded/unloaded
	// in battery_complication_state_changed)
	if(data->icon) {
		gdraw_command_image_draw(ctx, data->icon, GPointZero);
	}
}

BatteryComplication* battery_complication_create(GRect frame)
{
	Layer *layer = layer_create_with_data(frame, sizeof(BatteryComplicationData));
	layer_set_update_proc(layer, battery_complication_update);

	BatteryComplicationData *data = layer_get_data(layer);
	data->icon = NULL;
	data->animating = false;
	data->angle = 0;

	return (BatteryComplication*)layer;
}

void battery_complication_destroy(BatteryComplication *complication)
{
	Layer *layer = battery_complication_get_layer(complication);

	BatteryComplicationData *data = layer_get_data(layer);
	free(data->icon);

	layer_destroy(layer);
}

inline Layer* battery_complication_get_layer(BatteryComplication *complication)
{
	return (Layer*)complication;
}

void battery_complication_state_changed(BatteryComplication *complication,
                                        const BatteryChargeState *charge)
{
	Layer *layer = battery_complication_get_layer(complication);
	BatteryComplicationData *data = layer_get_data(layer);

	data->requested_state = *charge;

	if(!data->animating) {
		battery_complication_animate_to(complication, battery_complication_angle(charge));
	}

	// Update the icon
	free(data->icon);
	const uint32_t resource = charge->is_charging
		? RESOURCE_ID_BATTERY_CHARGING : RESOURCE_ID_BATTERY;
	data->icon = gdraw_command_image_create_with_resource(resource);

	layer_mark_dirty(layer);
}

static void battery_complication_spinup_animation_started(Animation *anim, void *context)
{
	BatteryComplication *complication = context;
	Layer *layer = battery_complication_get_layer(complication);
	BatteryComplicationData *data = layer_get_data(layer);

	data->animating = true;
}

static void battery_complication_spinup_animation_stopped(Animation *anim, bool finished, void *context)
{
	BatteryComplication *complication = context;
	Layer *layer = battery_complication_get_layer(complication);
	BatteryComplicationData *data = layer_get_data(layer);

	data->animating = false;

	// The only way this function will be called is if the animation has
	// finished, which can only be the case if
	// battery_complication_state_changed was called, so data->requested_state
	// will have been initialized.
	const uint32_t requested_angle = battery_complication_angle(&data->requested_state);

	if(requested_angle != data->angle) {
		// Re-animate to the correct angle.
		battery_complication_animate_to(complication, requested_angle);
	}
}

static uint32_t battery_complication_get_angle(void *subject)
{
	BatteryComplication *complication = subject;
	Layer *layer = battery_complication_get_layer(complication);
	BatteryComplicationData *data = layer_get_data(layer);

	return data->angle;
}

static void battery_complication_set_angle(void *subject, uint32_t angle)
{
	BatteryComplication *complication = subject;
	Layer *layer = battery_complication_get_layer(complication);
	BatteryComplicationData *data = layer_get_data(layer);

	data->angle = angle;
	layer_mark_dirty(layer);
}

static void battery_complication_animate_to(BatteryComplication *complication, uint32_t requested_angle)
{
	static const PropertyAnimationImplementation battery_spin_impl = {
		.base = {
			.update = (AnimationUpdateImplementation)property_animation_update_uint32
		},
		.accessors = {
			.setter = { .uint32 = battery_complication_set_angle },
			.getter = { .uint32 = battery_complication_get_angle },
		}
	};
	static const AnimationHandlers battery_spinup_anim_handlers = {
		.started = battery_complication_spinup_animation_started,
		.stopped = battery_complication_spinup_animation_stopped
	};

	uint32_t current_angle = battery_complication_get_angle(complication);
	PropertyAnimation *property_anim = property_animation_create(
		&battery_spin_impl,
		complication,
		NULL, NULL
	);
	property_animation_from(property_anim, &current_angle, sizeof(current_angle), true);
	property_animation_to(property_anim, &requested_angle, sizeof(requested_angle), true);
	Animation *anim = property_animation_get_animation(property_anim);
	animation_schedule(base_complication_setup_animation(anim, &battery_spinup_anim_handlers, complication));
}

