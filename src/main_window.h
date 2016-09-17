#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

extern "C" {
#include "pebble.h"
}

#include "boulder.h"

#include "variant.h"
#include "manual_animation.h"

#include "background_layer.h"
#include "complications/complication.h"
#include "face_layer.h"

#include "watcher/TimeWatcher.h"
#include "watcher/UnobstructedAreaWatcher.h"

#include <array>
#include <memory>

class MainWindow final
	: public Boulder::Window
	, private TimeCallback
	, private UnobstructedAreaCallback
{
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
#ifdef UNOBSTRUCTED_AREA
	using BoundsPAnim = Boulder::PropertyAnimation<
		Layer, GRect,
		layer_set_bounds, layer_get_bounds
	>;
	struct UnobstructedAnimations {
		ManualAnimation win_scoot;

		UnobstructedAnimations(ManualAnimation win_scoot)
			: win_scoot(win_scoot)
		{}
	};
	Variant<void, UnobstructedAnimations> area_animations;

	virtual void on_area_will_change(const GRect&  final_unobstructed_screen_area) override;
	virtual void on_area_change(AnimationProgress progress) override;
	virtual void on_area_did_change() override;

	std::pair<GRect, GRect> get_correct_frame_and_bounds(const GRect& unobstructed_area);
#endif

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

	static GRect top_complication_pos(GRect& size);
	static GRect bottom_complication_pos(GRect& size);

	static constexpr int16_t complication_size = 51;
	static constexpr int16_t complication_offset_x = PBL_IF_ROUND_ELSE(15, 10);
	static constexpr int16_t complication_offset_y = 15;
};

#endif
