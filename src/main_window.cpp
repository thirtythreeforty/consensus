#include "main_window.h"

#include "preferences.h"
#include "vibration.h"

MainWindow::MainWindow()
	: Window()
	, background_layer(get_bounds())
	, face_layer(get_bounds(), true)
	, complications_layer(get_bounds())
	, no_bluetooth_image(create_themed_bluetooth_bitmap())
{
}

MainWindow::~MainWindow()
{
	gbitmap_destroy(no_bluetooth_image);
}

void MainWindow::on_load()
{
	init_layers();
}

void MainWindow::on_unload()
{
	deinit_layers();
}

void MainWindow::init_layers()
{
	GRect size = get_bounds();
	GPoint center = grect_center_point(&size);

	add_child(background_layer);

	const GRect bluetooth_image_size = gbitmap_get_bounds(no_bluetooth_image);
	const GRect bluetooth_layer_location =
		GRect((int16_t)(center.x - bluetooth_image_size.size.w / 2),
		      (int16_t)(center.y - complication_offset_y - complication_size / 2 - bluetooth_image_size.size.h / 2),
		      (int16_t)bluetooth_image_size.size.w,
		      (int16_t)bluetooth_image_size.size.h);
	no_bluetooth_layer = bitmap_layer_create(bluetooth_layer_location);
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);
	bitmap_layer_set_compositing_mode(no_bluetooth_layer, GCompOpSet);
	// Immediately hide or show the icon
	update_connection_now();
	add_child(bitmap_layer_get_layer(no_bluetooth_layer));

	add_child(complications_layer);

	reinit_complications();

	face_layer.set_show_second(should_show_second());
	add_child(face_layer);
	animation_schedule(face_layer.animate_in(true, true));
}

void MainWindow::deinit_layers()
{
	bitmap_layer_destroy(no_bluetooth_layer);
}

void MainWindow::update_connection_now()
{
	on_connection_change(connection_service_peek_pebble_app_connection());
}

void MainWindow::configure()
{
	face_layer.set_show_second(should_show_second());
	background_layer.recolor();

	gbitmap_destroy(no_bluetooth_image);
	no_bluetooth_image = create_themed_bluetooth_bitmap();
	bitmap_layer_set_bitmap(no_bluetooth_layer, no_bluetooth_image);

	// The show-no-connection pref could have changed
	update_connection_now();

	reinit_complications();
}

void MainWindow::on_connection_change(bool connected)
{
	layer_set_hidden(bitmap_layer_get_layer(no_bluetooth_layer),
	                 connected || should_hide_no_bluetooth());
}

void MainWindow::on_battery_state_change(const BatteryChargeState& charge)
{
	complication_do<BatteryComplication>([&](auto& c){
		c.state_changed(charge);
	});
}


void MainWindow::on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	face_layer.set_time(tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);

	complication_do<DateComplication>([&](auto& c) {
		c.time_changed(tick_time);
	});
#ifdef PBL_HEALTH
	complication_do<HealthComplication>([&](auto& c) {
		c.on_tick(units_changed);
	});
#endif
	complication_do<TimeZoneComplication>([&](auto& c) {
		c.on_tick(units_changed);
	});

	// Vibrate once on the hour and twice at noon.
	if(tick_time->tm_min == 0 && tick_time->tm_sec == 0 &&
	   should_vibrate_on_hour() &&
	   vibration_ok()) {
		static const uint32_t vibe_pattern[] = {100, 250, 100};
		VibePattern vibe = {
			.durations = vibe_pattern,
			.num_segments = static_cast<unsigned int>(tick_time->tm_hour % 12 == 0 ? 3 : 1)
		};

		vibes_enqueue_custom_pattern(vibe);
	}
}

bool MainWindow::should_power_compass()
{
	return false;
}

void MainWindow::on_compass_power(bool on)
{
}

void MainWindow::on_compass_update(CompassHeadingData& heading)
{
}

#ifdef PBL_HEALTH
void MainWindow::on_health_update(HealthEventType event)
{
	complication_do<HealthComplication>([&](auto& c) {
		if(event == HealthEventSignificantUpdate) {
			c.on_significant_update();
		}
		else if(event == HealthEventMovementUpdate) {
			c.on_movement_update();
		}
	});
}
#endif

void MainWindow::reinit_complications()
{
	GRect size = complications_layer.get_frame();
	GPoint center = grect_center_point(&size);

	const GRect left_complication_position =
		GRect((int16_t)(center.x - complication_size - complication_offset_x),
		      (int16_t)(center.y - complication_size / 2),
		      (int16_t)complication_size,
		      (int16_t)complication_size);

	const GRect right_complication_position =
		GRect((int16_t)(center.x + complication_offset_x),
		      (int16_t)(center.y - complication_size / 2),
		      (int16_t)complication_size,
		      (int16_t)complication_size);

	const GRect bottom_complication_position =
		GRect((int16_t)(center.x - complication_size / 2),
		      (int16_t)(center.y + complication_offset_y),
		      (int16_t)complication_size,
		      (int16_t)complication_size);

	const std::array<std::pair<const GRect&, complication_config>, 3> complication_params = {{
		{ left_complication_position, left_complication_type() },
		{ bottom_complication_position, bottom_complication_type() },
		{ right_complication_position, right_complication_type() },
	}};
	// Make sure we don't walk off of the complications array if its size changes
	static_assert(std::tuple_size<decltype(complication_params)>::value ==
	              std::tuple_size<decltype(complications)>::value,
	              "Complication parameter array size mismatch");

	for(size_t i = 0; i < complication_params.size(); ++i) {
		using std::get;
		const GRect& position = get<0>(complication_params[i]);
		const auto& config = get<1>(complication_params[i]);
		const auto& type = get<0>(config);
		bool type_different = complications[i].type() != type;
		bool new_type_valid = AbstractComplication::typenum_of<void>() != type;

		if(type_different) {
			complications[i].emplace(type, position);
		}

		if(new_type_valid) {
			Complication& new_complication = complications[i].as<Complication>();

			new_complication.configure(get<1>(config));

			if(type_different) {
				// Type was changed, add the child
				complications_layer.add_child(new_complication);
			}
		}
	}

	const BatteryChargeState charge_state = battery_state_service_peek();
	on_battery_state_change(charge_state);

	WeatherData wdata = WeatherData::from_persist();
	complication_do<WeatherComplication>([&](auto& c) {
		c.weather_changed(wdata);
	});

	time_t time_s = time(nullptr);
	struct tm *time_struct = localtime(&time_s);
	TimeUnits all_units_changed = TimeUnits(
		SECOND_UNIT |
		MINUTE_UNIT |
		HOUR_UNIT |
		DAY_UNIT |
		MONTH_UNIT |
		YEAR_UNIT
	);

	complication_do<DateComplication>([&](auto& c) {
		c.time_changed(time_struct);
	});
#ifdef PBL_HEALTH
	complication_do<HealthComplication>([&](auto& c) {
		c.on_tick(all_units_changed);
	});
#endif
	complication_do<TimeZoneComplication>([&](auto& c) {
		c.on_tick(all_units_changed);
	});
}

GBitmap* MainWindow::create_themed_bluetooth_bitmap()
{
	return gbitmap_create_with_resource(theme().no_bluetooth_resource());
}
