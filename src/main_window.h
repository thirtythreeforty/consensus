#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

extern "C" {
#include "pebble.h"
}

#include "boulder.h"

#include "background_layer.h"
#include "complications/complication.h"
#include "face_layer.h"

#include "watcher/TimeWatcher.h"

#include <array>
#include <memory>

class MainWindow : public Boulder::Window, private TimeCallback {
	BackgroundLayer background_layer;
	FaceLayer face_layer;
	Boulder::Layer complications_layer;
	std::array<AbstractComplication, 4> complications;

public:
	explicit MainWindow();
	~MainWindow() = default;

	void configure();

	void on_connection_change(bool connected);

	virtual void on_tick(struct tm *tick_time, TimeUnits units_changed) override;

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
