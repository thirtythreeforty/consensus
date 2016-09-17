#include "main_window.h"

#include "AnimationBuffer.h"
#include "preferences.h"
#include "vibration.h"

MainWindow::MainWindow()
	: Window()
	, TimeCallback(should_show_second() ? SECOND_UNIT : MINUTE_UNIT)
	, background_layer(get_bounds())
	, face_layer(get_bounds(), true)
	, complications_layer(get_bounds())
{
	set_background_color(theme().background_color());
	update_time_now();
}

void MainWindow::on_load()
{
	init_layers();

#ifdef UNOBSTRUCTED_AREA
	Layer *root_layer = get_root_layer();

	GRect win_bounds, win_frame;
	std::tie(win_frame, win_bounds) = get_correct_frame_and_bounds(layer_get_unobstructed_bounds(root_layer));

	layer_set_frame(root_layer, win_frame);
	layer_set_bounds(root_layer, win_bounds);
#endif
}

void MainWindow::on_unload()
{
	deinit_layers();
}

void MainWindow::init_layers()
{
	add_child(background_layer);
	add_child(complications_layer);

	reinit_complications();

	face_layer.set_show_second(should_show_second());
	add_child(face_layer);
}

void MainWindow::deinit_layers()
{
}

void MainWindow::configure()
{
	face_layer.set_show_second(should_show_second());
	set_background_color(theme().background_color());
	background_layer.recolor();

	update_time_subscription(should_show_second() ? SECOND_UNIT : MINUTE_UNIT);
	update_time_now();

	reinit_complications();
}

void MainWindow::on_tick(struct tm *tick_time, TimeUnits units_changed)
{
	face_layer.set_time(tick_time->tm_hour, tick_time->tm_min, tick_time->tm_sec);
}

#ifdef UNOBSTRUCTED_AREA
void MainWindow::on_area_will_change(const GRect& final_unobstructed_screen_area)
{
	// Scoot the face to remain centered.
	GRect win_bounds, win_frame;
	std::tie(win_frame, win_bounds) = get_correct_frame_and_bounds(final_unobstructed_screen_area);

	Layer *root_layer = get_root_layer();

	area_animations.emplace_as<UnobstructedAnimations>(
		ManualAnimation(BoundsPAnim(*root_layer, nullptr, &win_bounds).release())
	);

	layer_set_frame(root_layer, win_frame);
}

void MainWindow::on_area_change(AnimationProgress progress)
{
	// Forward the event to the animations in progress.
	area_animations.as<UnobstructedAnimations>().win_scoot.update(progress);
}

void MainWindow::on_area_did_change()
{
	area_animations.reset();
}

std::pair<GRect, GRect> MainWindow::get_correct_frame_and_bounds(const GRect& unobstructed_area)
{
	std::pair<GRect, GRect> frame_and_bounds;
	GRect &win_frame = std::get<0>(frame_and_bounds);
	GRect &win_bounds = std::get<1>(frame_and_bounds);

	Layer *root_layer = get_root_layer();
	win_frame = layer_get_frame(root_layer);
	win_bounds = layer_get_bounds(root_layer);

	const int16_t size_difference = (unobstructed_area.size.h - win_frame.size.h);
	const int16_t scoot_distance = size_difference / 2;
	win_bounds.origin.y += scoot_distance;
	win_bounds.size.h -= size_difference;
	win_frame.size.h += size_difference;

	return frame_and_bounds;
}
#endif

void MainWindow::reinit_complications()
{
	GRect size = complications_layer.get_frame();
	GPoint center = grect_center_point(&size);

	const GRect top_complication_position = top_complication_pos(size);

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

	const GRect bottom_complication_position = bottom_complication_pos(size);

	const std::array<std::pair<const GRect&, complication_config>, 4> complication_params = {{
		{ top_complication_position, top_complication_type() },
		{ left_complication_position, left_complication_type() },
		{ bottom_complication_position, bottom_complication_type() },
		{ right_complication_position, right_complication_type() },
	}};
	// Make sure we don't walk off of the complications array if its size changes
	static_assert(std::tuple_size<decltype(complication_params)>::value ==
	              std::tuple_size<decltype(complications)>::value,
	              "Complication parameter array size mismatch");

	bool show_low_batt = true;
	for(size_t i = 0; i < complication_params.size(); ++i) {
		using std::get;
		const GRect& position = get<0>(complication_params[i]);
		const auto& config = get<1>(complication_params[i]);
		const auto& type = get<0>(config);
		bool type_different = complications[i].type() != type;
		bool new_type_valid = AbstractComplication::typenum_of<void>() != type;

		if(AbstractComplication::typenum_of<BatteryComplication>() == type) {
			show_low_batt = false;
		}

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

	WeatherData wdata = WeatherData::from_persist();
	complication_do<WeatherComplication>([&](auto& c) {
		c.weather_changed(wdata);
	});

	complication_do<StatusComplication>([&](auto& c) {
		c.enable_battery_alert(show_low_batt);
	});
}

GRect MainWindow::top_complication_pos(GRect& size)
{
	const GPoint center = grect_center_point(&size);
	return GRect((int16_t)(center.x - complication_size / 2),
	             (int16_t)(center.y - complication_size - complication_offset_y),
	             (int16_t)complication_size,
	             (int16_t)complication_size);
}

GRect MainWindow::bottom_complication_pos(GRect& size)
{
	const GPoint center = grect_center_point(&size);
	return GRect((int16_t)(center.x - complication_size / 2),
				 (int16_t)(center.y + complication_offset_y),
				 (int16_t)complication_size,
				 (int16_t)complication_size);
}
