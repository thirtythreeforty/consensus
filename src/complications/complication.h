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

#include "watcher/BatteryWatcher.h"
#include "watcher/ConnectionWatcher.h"
#include "watcher/HealthWatcher.h"
#include "watcher/TimeWatcher.h"

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
	Boulder::TextLayer unit_text;
	LazyIcon icon;

public:
	virtual void configure(const config_bundle_t& config) override;

protected:
	IconTextComplication(GRect frame);

	virtual void update(GContext* ctx) override;
	void icontext_update(GContext* ctx);

	void set_icon(uint32_t resource_id);
	void reset_icon();

	void set_number(int32_t n);
	void set_number_format(const char* fmt, int32_t n, const char* unit);
	void reset_number();

	constexpr static const char* plain_number_format = "%i";
	constexpr static const char* empty = "";

private:
	static GRect calculate_unit_frame(const GRect& complication_frame);
};

class HighlightComplication: public IconTextComplication, protected AnimatedCallback
{
	friend class HighlightComplication2;
	using angle_t = int32_t;

	static bool angles_are_close(const angle_t& a, const angle_t& b);
	static angle_t clamp_angle(const angle_t& angle);

protected:
	using AnimatedAngle = Animated<angle_t, angles_are_close, clamp_angle>;
	AnimatedAngle angle;

protected:
	HighlightComplication(GRect frame);

	virtual void update(GContext* ctx) override;

	virtual GColor highlight_color() const = 0;

private:
	virtual void on_animated_update(void *animated) override;
};

class TickComplication: public HighlightComplication
{
protected:
	AnimatedAngle tick_angle;

	TickComplication(GRect frame);

	virtual void update(GContext* ctx) override;
	virtual GColor tick_color() const = 0;
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

class DateComplication final: public IconTextComplication, private TimeCallback
{
	typedef enum {
		DayOfWk = 0,
		Month,
		None
	} UnitType;
	UnitType unit_type;

public:
	DateComplication(GRect frame);
	~DateComplication() = default;

	virtual void configure(const config_bundle_t&) override;

	virtual void on_tick(struct tm *time, TimeUnits) override;

private:
	const char* unit(struct tm *time);
	static const char* weekday(int wday);
	static const char* month(int m);
};

class TimeZoneComplication final: public Complication, private TimeCallback
{
	FaceLayer face;
	int32_t offset_sec;

public:
	TimeZoneComplication(GRect frame);
	~TimeZoneComplication() = default;

	virtual void configure(const config_bundle_t&) override;

	virtual void on_tick(struct tm *tick_time, TimeUnits units_changed) override;

private:
	void update_time();
};

class WeatherComplication final: public HighlightComplication2
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

	constexpr static const char* relhum_format = plain_number_format;

	constexpr static const char* degf_unit = "\u00B0F";
	constexpr static const char* degc_unit = "\u00B0C";
	constexpr static const char* percent_unit = "%";
};

class BatteryComplication final: public HighlightComplication, private BatteryCallback
{
public:
	explicit BatteryComplication(GRect frame);

	virtual void on_battery_change(const BatteryChargeState& state) override;

protected:
	GColor highlight_color() const override;

private:
	static uint32_t calc_angle(const BatteryChargeState& state);
};

class HealthComplication final
	: public TickComplication
#ifdef PBL_HEALTH
	, private HealthCallback
	, private TimeCallback
#endif
{
	struct Goal {
		int32_t goal;
		Goal(int32_t goal) : goal(goal) {}
	};
	struct AutoGoal : public Goal { using Goal::Goal; };
	struct ManualGoal : public Goal { using Goal::Goal; };

	using divisor_t = std::tuple<unsigned int, unsigned int, const char*, const char*>;
	template<int N> using divisor_list_t = std::array<divisor_t, N>;

	struct Metric {
		int32_t metric;
		const char* unit;
		const char* format;
		Metric(int32_t metric, const char* unit, const char* format)
			: metric(metric), unit(unit), format(format)
		{}
	};

	enum GadgetType {
		ICON = 0,
		STEPS,
		DISTANCE,
		CALORIES_ACTIVE,
		CALORIES_RESTING,
		CALORIES_TOTAL,
		ACTIVE_SECONDS,
	};

	// Only need these members on watches that will actually use it
#ifdef PBL_HEALTH
	GadgetType gadget_type;

	Variant<void, AutoGoal, ManualGoal> step_goal;
	Variant<void, Metric> today_metric;

	static Variant<void, int32_t> st_today_steps;
	static Variant<void, int32_t> st_today_average_steps;
#endif

public:
	explicit HealthComplication(GRect frame);
	~HealthComplication() = default;

	virtual void configure(const config_bundle_t& config) override;

#ifdef PBL_HEALTH
	virtual void on_tick(struct tm*, TimeUnits units_changed) override;

	virtual void on_movement_update() override;
	virtual void on_significant_update() override;
#endif

protected:
	virtual GColor highlight_color() const override;
	virtual GColor tick_color() const override;

private:
	void recalculate_today_data();
	void invalidate_today_data();

	void store_scaled_metric(uint32_t metric, const divisor_t* begin, const divisor_t* end);

	template<typename T>
	void store_scaled_metric(uint32_t metric, const T& t) {
		store_scaled_metric(metric, t.begin(), t.end());
	}

	void recalculate_auto_goal();

	void update_angle_and_gadget();

	static void refresh_static_steps_today();
	static void invalidate_static_today_data();

	static const divisor_list_t<3> time_divisors;
	static const divisor_list_t<3> imperial_distance_divisors;
	static const divisor_list_t<3> metric_distance_divisors;

	constexpr static const char *point_number_format = "0.%i";
	constexpr static const char *steps_unit = "STEPS";
};

class StatusComplication final
	: public Complication
	, private BatteryCallback
	, private ConnectionCallback
{
	BitmapLayer *no_bluetooth_layer;
	GBitmap *no_bluetooth_image;

	BitmapLayer *battery_layer;
	GBitmap *low_battery_image;
	GBitmap *chg_battery_image;
	bool show_battery_alert;

public:
	explicit StatusComplication(GRect frame);
	~StatusComplication();

	virtual void configure(const config_bundle_t& config) override;

	void enable_battery_alert(bool enable);

protected:
	virtual void update(GContext* ctx) override;

private:
	virtual void on_connection_change(bool connected) override;
	virtual void on_battery_change(const BatteryChargeState&) override;

	static GBitmap* create_themed_bluetooth_bitmap();
	static GBitmap* create_themed_low_batt_bitmap();
	static GBitmap* create_themed_chg_batt_bitmap();
};

using AbstractComplication = Variant<
	// These must be kept in this order to preserve the mapping in the config
	void,
	BatteryComplication,
	DateComplication,
	WeatherComplication,
	HealthComplication,
	TimeZoneComplication,
	StatusComplication
>;

#endif
