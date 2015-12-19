#include "complication_common.h"

#include "common.h"

typedef struct {
	BatteryChargeState charge_state;
	bool animating;
	int32_t animation_angle;
	GDrawCommandImage *icon;
} BatteryComplicationData;

static int32_t battery_complication_angle(uint8_t charge_percent)
{
	return charge_percent * TRIG_MAX_ANGLE / 100;
}

void battery_complication_update(Layer *layer, GContext *ctx)
{
	BatteryComplicationData *data = layer_get_data(layer);

	int32_t angle = data->animating ? data->animation_angle
	                                : battery_complication_angle(data->charge_state.charge_percent);

	base_complication_update(layer, ctx, GColorChromeYellow, angle);

	// Draw the icon based on the charging state (icon is loaded/unloaded
	// in battery_complication_state_changed)
	gdraw_command_image_draw(ctx, data->icon, GPointZero);
}

BatteryComplication* battery_complication_create(GRect bounds, BatteryChargeState *charge)
{
	Layer *layer = layer_create_with_data(bounds, sizeof(BatteryComplicationData));
	layer_set_update_proc(layer, battery_complication_update);

	BatteryComplicationData *data = layer_get_data(layer);
	data->icon = NULL;
	data->animating = true; // Must call battery_complication_animate_in

	battery_complication_state_changed((BatteryComplication*)layer, charge);

	return (BatteryComplication*)layer;
}

void battery_complication_destroy(BatteryComplication *complication)
{
	Layer *layer = (Layer*)complication;

	BatteryComplicationData *data = layer_get_data(layer);
	free(data->icon);

	layer_destroy(layer);
}

Layer* battery_complication_get_layer(BatteryComplication *complication)
{
	return (Layer*)complication;
}

void battery_complication_state_changed(BatteryComplication *complication,
                                        BatteryChargeState *charge)
{
	Layer *layer = (Layer*)complication;
	BatteryComplicationData *data = layer_get_data(layer);

	data->charge_state = *charge;

	// Update the icon
	free(data->icon);
	const uint32_t resource = data->charge_state.is_charging
		? RESOURCE_ID_BATTERY_CHARGING : RESOURCE_ID_BATTERY;
	data->icon = gdraw_command_image_create_with_resource(resource);

	layer_mark_dirty(layer);
}

void battery_complication_animate_in(BatteryComplication *complication)
{
	Layer *layer = (Layer*)complication;
	BatteryComplicationData *data = layer_get_data(layer);

	// TODO
	data->animating = false;
}
