#include "complication.h"

StatusComplication::StatusComplication(GRect frame)
	: Complication(frame)
	, no_bluetooth_image(create_themed_bluetooth_bitmap())
{
	GRect size = get_bounds();
	GPoint center = grect_center_point(&size);

	const GRect bluetooth_image_size = gbitmap_get_bounds(no_bluetooth_image);
	const GRect bluetooth_layer_location =
		GRect((int16_t)(center.x - bluetooth_image_size.size.w / 2),
		      (int16_t)(center.y - bluetooth_image_size.size.h / 2),
		      (int16_t)bluetooth_image_size.size.w,
		      (int16_t)bluetooth_image_size.size.h);
	no_bluetooth_layer = bitmap_layer_create(bluetooth_layer_location);
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);
	bitmap_layer_set_compositing_mode(no_bluetooth_layer, GCompOpSet);
	add_child(bitmap_layer_get_layer(no_bluetooth_layer));

	// Immediately hide or show the icon
	update_connection_now();
}

StatusComplication::~StatusComplication()
{
	bitmap_layer_destroy(no_bluetooth_layer);
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

	// The show-no-connection pref could have changed
	update_connection_now();
}

GBitmap* StatusComplication::create_themed_bluetooth_bitmap()
{
	return gbitmap_create_with_resource(theme().no_bluetooth_resource());
}

void StatusComplication::update_connection_now()
{
	on_connection_change(connection_service_peek_pebble_app_connection());
}

void StatusComplication::on_connection_change(bool connected)
{
	layer_set_hidden(bitmap_layer_get_layer(no_bluetooth_layer),
	                 connected || should_hide_no_bluetooth());
}
