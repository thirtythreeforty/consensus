#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

extern "C" {
#include "pebble.h"
}

#include "boulder.h"

#include "background_layer.h"
#include "complications/complication.h"
#include "face_layer.h"

#include <array>
#include <memory>

class MainWindow : public Boulder::Window {
	BackgroundLayer background_layer;
	FaceLayer face_layer;
	Boulder::Layer complications_layer;
	std::array<AbstractComplication, 3> complications;

	BitmapLayer *no_bluetooth_layer;
	GBitmap *no_bluetooth_image;

public:
	explicit MainWindow();
	~MainWindow();

	void configure();

	void on_connection_change(bool connected);
	void on_battery_state_change(const BatteryChargeState& charge);
	void on_tick(struct tm *tick_time, TimeUnits units_changed);

	bool should_power_compass();
	void on_compass_power(bool on);
	void on_compass_update(CompassHeadingData& heading);

#ifdef PBL_HEALTH
	void on_health_update(HealthEventType event);
#endif

protected:
	void on_load() override;
	void on_unload() override;

private:
	void init_layers();
	void deinit_layers();

	void update_connection_now();

	void reinit_complications();

	static GBitmap* create_themed_bluetooth_bitmap();

	template<typename C, typename F>
	void complication_do(const F& f)
	{
		for(auto& c: complications) {
			c.if_is<C>(f);
		};
	}

	static constexpr int16_t complication_size = 51;
	static constexpr int16_t complication_offset_x = PBL_IF_ROUND_ELSE(15, 10);
	static constexpr int16_t complication_offset_y = 15;
};

#endif
