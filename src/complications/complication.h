#ifndef COMPLICATION_H
#define COMPLICATION_H

extern "C" {
#include <pebble.h>
}

typedef struct {
	bool valid;

	time_t time_updated;
	int8_t temp_c;
	uint8_t humidity;
	uint8_t icon;
} WeatherData;

void weather_from_appmessage(DictionaryIterator *iter, WeatherData *wdata);
void weather_from_persist(WeatherData *wdata);
void weather_to_persist(const WeatherData *wdata);

#include <experimental/optional>

#include "boulder.h"
#include "lazy_icon.h"
#include "ScrambledNumber.h"

class Complication: public Boulder::Layer
{
public:
	~Complication() = default;

	virtual void configure(const std::array<unsigned int, 4>& config);

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
	virtual void configure(const std::array<unsigned int, 4>& config) override;

protected:
	IconTextComplication(GRect frame);

	virtual void update(GContext* ctx) override;

	void set_icon(uint32_t resource_id);
	void reset_icon();

	void set_number(int32_t n);
	void set_number_format(const char* fmt, int32_t n);
};

class HighlightComplication: public IconTextComplication
{
	friend class HighlightComplication2;
	using angle_t = int32_t;

	bool animating = false;
	angle_t requested_angle = 0, angle = 0;

protected:
	using IconTextComplication::IconTextComplication;

	virtual void update(GContext* ctx) override;

	void set_angle(angle_t new_angle);
	inline const angle_t& get_angle() { return requested_angle; }

	virtual GColor highlight_color() const = 0;

private:
	// Helpers for animations
	void animate_to_requested();
	static void panim_set_angle(HighlightComplication& complication, const angle_t& new_angle);
	static const angle_t& panim_get_angle(const HighlightComplication& complication);
	static void panim_stopped(Animation *anim, bool finished, void *context);
};

class HighlightComplication2: public HighlightComplication
{
	bool animating2 = false;
	angle_t requested_angle2 = 0, angle2 = 0;

protected:
	using HighlightComplication::HighlightComplication;

	virtual void update(GContext* ctx) override;

	void set_angle2(angle_t new_angle2);
	inline const angle_t& get_angle2() { return requested_angle2; }

	virtual GColor highlight_color2() const = 0;

private:
	// The '2' versions of all these functions are kinda gross, but they all
	// refer to angle2 of this class.
	static void panim_set_angle2(HighlightComplication2& complication, const angle_t& new_angle);
	static const angle_t& panim_get_angle2(const HighlightComplication2& complication);
	void animate_to_requested2();
	static void panim_stopped2(Animation *anim, bool finished, void *context);
};

class DateComplication: public Complication
{
	ScrambledNumber date;

public:
	DateComplication(GRect frame);
	~DateComplication() = default;

	virtual void configure(const std::array<unsigned int, 4>&) override;

	void time_changed(struct tm *time);
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

	std::experimental::optional<Boulder::AppTimer> refresh_timer;

public:
	explicit WeatherComplication(GRect frame);
	~WeatherComplication() = default;

	void weather_changed(const WeatherData &new_weather);

	virtual void configure(const std::array<unsigned int, 4>& config) override;

protected:
	virtual GColor highlight_color() const override;
	virtual GColor highlight_color2() const override;

private:
	void schedule_refresh(time_t last_refresh_time);

	static WeatherAngles compute_angles(const WeatherData& wdata);
	static void request_refresh(void*);

	static const char* deg_format;
	static const char* relhum_format;
	static const char* empty_format;
};

class BatteryComplication: public HighlightComplication
{
public:
	explicit BatteryComplication(GRect frame);

	void state_changed(const BatteryChargeState *state);

protected:
	GColor highlight_color() const override;
};

class HealthComplication: public HighlightComplication
{
	std::experimental::optional<uint32_t> average_steps;

public:
	explicit HealthComplication(GRect frame);
	~HealthComplication();

	void on_movement_update();
	void on_significant_update();

protected:
	virtual GColor highlight_color() const override;

private:
	void recalculate_average_steps();
};

template<typename T> constexpr uint8_t complication_type_map;
template<> constexpr uint8_t complication_type_map<void> = 0;
template<> constexpr uint8_t complication_type_map<BatteryComplication> = 1;
template<> constexpr uint8_t complication_type_map<DateComplication> = 2;
template<> constexpr uint8_t complication_type_map<WeatherComplication> = 3;
template<> constexpr uint8_t complication_type_map<HealthComplication> = 4;

class AbstractComplication
{
	Complication *complication;
	uint8_t _type;

	template<typename T>
	AbstractComplication(T *ptr)
		: complication(ptr)
		, _type(complication_type_map<T>)
	{}

public:
	AbstractComplication() : _type(complication_type_map<void>) {}

	template<typename T>
	auto downcast() -> T* {
		if(_type == complication_type_map<T>) {
			return static_cast<T*>(complication);
		}
		else {
			return nullptr;
		}
	}

	bool valid() { return _type != complication_type_map<void>; }

	operator Complication&() {
		return *complication;
	}

	auto type() { return _type; }

	static AbstractComplication create(unsigned int type, const GRect& frame) {
		switch(type) {
		case complication_type_map<void>:
			return {};
		case complication_type_map<BatteryComplication>:
			return {new BatteryComplication(frame)};
		case complication_type_map<DateComplication>:
			return {new DateComplication(frame)};
		case complication_type_map<WeatherComplication>:
			return {new WeatherComplication(frame)};
		case complication_type_map<HealthComplication>:
			return {new HealthComplication(frame)};
		default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Asked for bad complication type %i", type);
			return {};
		}
	}

	bool change_type(unsigned int type, const GRect& frame) {
		if(type == this->_type) {
			return false;
		}
		destroy();
		*this = create(type, frame);
		return valid();
	}

	void destroy() {
		if(_type != complication_type_map<void>) {
			delete complication;
			_type = complication_type_map<void>;
		}
	}
};

#endif
