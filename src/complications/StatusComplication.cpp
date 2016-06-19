#include "complication.h"

StatusComplication::StatusComplication(GRect frame)
	: Complication(frame)
	, no_bluetooth_image(create_themed_bluetooth_bitmap())
	, low_battery_image(create_themed_low_batt_bitmap())
	, chg_battery_image(create_themed_chg_batt_bitmap())
	, show_battery_alert(true)
{
	GRect size = get_bounds();
	GPoint center = grect_center_point(&size);

	const GRect bluetooth_image_size = gbitmap_get_bounds(no_bluetooth_image);
	// Assume that the low battery image and the charging images are the same
	// size, which should be always true.
	const GRect batt_image_size = gbitmap_get_bounds(low_battery_image);

	const GRect bluetooth_layer_location =
		GRect((int16_t)(center.x - bluetooth_image_size.size.w / 2),
		      (int16_t)((center.y - bluetooth_image_size.size.h) / 2),
		      (int16_t)bluetooth_image_size.size.w,
		      (int16_t)bluetooth_image_size.size.h);
	no_bluetooth_layer = bitmap_layer_create(bluetooth_layer_location);
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);
	bitmap_layer_set_compositing_mode(no_bluetooth_layer, GCompOpSet);
	add_child(bitmap_layer_get_layer(no_bluetooth_layer));

	const GRect batt_layer_location =
		GRect((int16_t)(center.x - batt_image_size.size.w / 2),
		      (int16_t)(center.y / 2 + center.y - batt_image_size.size.h / 2),
		      (int16_t)batt_image_size.size.w,
		      (int16_t)batt_image_size.size.h);
	battery_layer = bitmap_layer_create(batt_layer_location);
	bitmap_layer_set_compositing_mode(battery_layer, GCompOpSet);
	// Wait to set the bitmap until a battery update happens
	add_child(bitmap_layer_get_layer(battery_layer));

	// Immediately hide or show the icons
	update_connection_now();
	update_battery_now();
}

StatusComplication::~StatusComplication()
{
	bitmap_layer_destroy(battery_layer);
	bitmap_layer_destroy(no_bluetooth_layer);
	gbitmap_destroy(low_battery_image);
	gbitmap_destroy(chg_battery_image);
	gbitmap_destroy(no_bluetooth_image);
}

void StatusComplication::update(GContext* ctx)
{
	// Do nothing. Especially do not call the superclass, because we do not
	// want to draw the ring.
}

void StatusComplication::configure(const config_bundle_t& config)
{
	gbitmap_destroy(no_bluetooth_image);
	no_bluetooth_image = create_themed_bluetooth_bitmap();
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);

	gbitmap_destroy(low_battery_image);
	low_battery_image = create_themed_low_batt_bitmap();
	gbitmap_destroy(chg_battery_image);
	chg_battery_image = create_themed_chg_batt_bitmap();
	// The show-no-connection pref could have changed
	update_connection_now();
	update_battery_now();
}

void StatusComplication::enable_battery_alert(bool enable)
{
	show_battery_alert = enable;
	update_battery_now();
}

GBitmap* StatusComplication::create_themed_bluetooth_bitmap()
{
	return gbitmap_create_with_resource(theme().no_bluetooth_resource());
}

GBitmap* StatusComplication::create_themed_low_batt_bitmap()
{
	return gbitmap_create_with_resource(theme().low_batt_resource());
}

GBitmap* StatusComplication::create_themed_chg_batt_bitmap()
{
	return gbitmap_create_with_resource(theme().chg_batt_resource());
}

void StatusComplication::on_connection_change(bool connected)
{
	layer_set_hidden(bitmap_layer_get_layer(no_bluetooth_layer),
	                 connected || should_hide_no_bluetooth());
}

void StatusComplication::on_battery_change(const BatteryChargeState& state)
{
	if(show_battery_alert && state.is_charging) {
		bitmap_layer_set_bitmap(battery_layer, chg_battery_image);
		layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
	}
	else if(show_battery_alert && state.charge_percent <= 20) {
		bitmap_layer_set_bitmap(battery_layer, low_battery_image);
		layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
	}
	else {
		layer_set_hidden(bitmap_layer_get_layer(battery_layer), true);
	}
}

