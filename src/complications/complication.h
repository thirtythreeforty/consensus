#ifndef COMPLICATION_H
#define COMPLICATION_H

extern "C" {
#include <pebble.h>
}

struct WeatherData {
	bool valid;

	time_t time_updated;
	int8_t temp_c;
	uint8_t humidity;
	uint8_t icon;

	void to_persist();

	static WeatherData from_appmessage(DictionaryIterator *iterator);
	static WeatherData from_persist();
};

#include "animated.h"
#include "boulder.h"
#include "face_layer.h"
#include "lazy_icon.h"
#include "ScrambledNumber.h"
#include "variant.h"

class Complication: public Boulder::Layer
{
public:
	using config_bundle_t = std::array<unsigned int, 4>;
	~Complication() = default;

	virtual void configure(const config_bundle_t& config);

protected:
	explicit Complication(GRect frame) : Boulder::Layer(frame) {}

	virtual void update(GContext* ctx) override;

	void base_setup_animation(Animation *anim, const AnimationHandlers &handlers);
};

class IconTextComplication: public Complication
{
	ScrambledNumber number;
	LazyIcon icon;

public:
	virtual void configure(const config_bundle_t& config) override;

protected:
	IconTextComplication(GRect frame);

	virtual void update(GContext* ctx) override;

	void set_icon(uint32_t resource_id);
	void reset_icon();

	void set_number(int32_t n);
	void set_number_format(const char* fmt, int32_t n);
};

class HighlightComplication: public IconTextComplication, private AnimatedCallback
{
	friend class HighlightComplication2;
	using angle_t = int32_t;

	static bool angles_are_close(const angle_t& a, const angle_t& b);
	static angle_t clamp_angle(const angle_t& angle);
	using AnimatedAngle = Animated<angle_t, angles_are_close, clamp_angle>;

protected:
	AnimatedAngle angle;

protected:
	HighlightComplication(GRect frame);

	virtual void update(GContext* ctx) override;

	virtual GColor highlight_color() const = 0;

private:
	virtual void on_animated_update() override;
};

class HighlightComplication2: public HighlightComplication
{
protected:
	AnimatedAngle angle2;

protected:
	HighlightComplication2(GRect frame);

	virtual void update(GContext* ctx) override;

	virtual GColor highlight_color2() const = 0;
};

class DateComplication: public Complication
{
	ScrambledNumber date;

public:
	DateComplication(GRect frame);
	~DateComplication() = default;

	virtual void configure(const config_bundle_t&) override;

	void time_changed(struct tm *time);
};

class TimeZoneComplication: public Complication
{
	FaceLayer face;
	int32_t offset_sec;

public:
	TimeZoneComplication(GRect frame);
	~TimeZoneComplication() = default;

	virtual void configure(const config_bundle_t&) override;

	void on_tick(TimeUnits units_changed);

private:
	void update_time();
};

class CompassComplication: public IconTextComplication
{
public:
	CompassComplication(GRect frame);

	virtual void configure(const config_bundle_t& config) override;

	void on_power(bool on);
	void on_update(CompassHeadingData& heading);

protected:
	virtual void update(GContext* ctx) override;
};

class WeatherComplication: public HighlightComplication2
{
	struct WeatherAngles {
		int32_t temp_angle;
		int32_t humidity_angle;
	};

	enum GadgetType {
		ICON = 0,
		TEMP_C,
		TEMP_F,
		RELHUM,
	};
	GadgetType gadget_type;

public:
	explicit WeatherComplication(GRect frame);
	~WeatherComplication() = default;

	void weather_changed(const WeatherData &new_weather);

	virtual void configure(const config_bundle_t& config) override;

protected:
	virtual GColor highlight_color() const override;
	virtual GColor highlight_color2() const override;

private:
	static void schedule_refresh(time_t last_refresh_time);

	static WeatherAngles compute_angles(const WeatherData& wdata);

	static const char* deg_format;
	static const char* relhum_format;
	static const char* empty_format;
};

class BatteryComplication: public HighlightComplication
{
public:
	explicit BatteryComplication(GRect frame);

	void state_changed(const BatteryChargeState& state);

protected:
	GColor highlight_color() const override;
};

class HealthComplication: public HighlightComplication
{
	struct Goal {
		int32_t goal;
		Goal(int32_t goal) : goal(goal) {}
	};
	struct AutoGoal : public Goal { using Goal::Goal; };
	struct ManualGoal : public Goal { using Goal::Goal; };

	// Only need these members on watches that will actually use it
#ifdef PBL_HEALTH

	Variant<void, AutoGoal, ManualGoal> step_goal;
	static Variant<void, int32_t> today_steps;

#endif

public:
	explicit HealthComplication(GRect frame);
	~HealthComplication() = default;

	virtual void configure(const config_bundle_t& config) override;

	void on_tick(TimeUnits units_changed);
	void on_movement_update();
	void on_significant_update();

protected:
	virtual GColor highlight_color() const override;

private:
	void recalculate_average_steps();
	static void refresh_steps_today();
	static void invalidate_steps_today();
	void update_angle_and_icon();
};

using AbstractComplication = Variant<
	// These must be kept in this order to preserve the mapping in the config
	void,
	BatteryComplication,
	DateComplication,
	WeatherComplication,
	HealthComplication,
	TimeZoneComplication,
	CompassComplication
>;

#endif
