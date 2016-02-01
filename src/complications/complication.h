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

class Complication: public Boulder::Layer
{
protected:
	explicit Complication(GRect frame) : Boulder::Layer(frame) {}
	~Complication() = default;

	virtual void update(GContext* ctx) override;

	void base_setup_animation(Animation *anim, const AnimationHandlers &handlers);
};

class HighlightComplication: public Complication
{
	friend class HighlightComplication2;
	using angle_t = int32_t;

	bool animating = false;
	angle_t requested_angle = 0, angle = 0;

protected:
	using Complication::Complication;

	virtual void update(GContext* ctx) override;

	void set_angle(angle_t new_angle);

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
	bool animating;
	uint8_t anim_frames_skipped;

	uint8_t requested_date;

	Boulder::TextLayer date_layer;
	std::array<char, 3> date_layer_text;

public:
	DateComplication(GRect frame);
	~DateComplication() = default;

	Animation* animate_in();

	void time_changed(struct tm *time);

private:
	GRect calculate_date_frame();
	void set_displayed(uint8_t mday);

	static void spin_animation_started(Animation *animation, void *context);
	static void spin_animation_stopped(Animation *animation, bool finished, void *context);
	static void spin_animation_update(Animation* anim, AnimationProgress progress);
};

class WeatherComplication: public HighlightComplication2
{
	struct WeatherAngles {
		int32_t temp_angle;
		int32_t humidity_angle;
	};

	std::experimental::optional<Boulder::GDrawCommandImage> icon;
	GPoint icon_shift;

	std::experimental::optional<Boulder::AppTimer> refresh_timer;

public:
	explicit WeatherComplication(GRect frame);
	~WeatherComplication() = default;

	void weather_changed(const WeatherData &new_weather);

protected:
	void update(GContext *ctx) override;
	virtual GColor highlight_color() const override;
	virtual GColor highlight_color2() const override;

private:
	void schedule_refresh(time_t last_refresh_time);

	static WeatherAngles compute_angles(const WeatherData& wdata);
	static void request_refresh(void*);
};

class BatteryComplication: public HighlightComplication
{
	std::experimental::optional<Boulder::GDrawCommandImage> icon;

public:
	explicit BatteryComplication(GRect frame);

	void state_changed(const BatteryChargeState *state);

protected:
	void update(GContext *ctx) override;
	GColor highlight_color() const override;
};

template<typename T> constexpr uint8_t complication_type_map;
template<> constexpr uint8_t complication_type_map<void> = 0;
template<> constexpr uint8_t complication_type_map<BatteryComplication> = 1;
template<> constexpr uint8_t complication_type_map<DateComplication> = 2;
template<> constexpr uint8_t complication_type_map<WeatherComplication> = 3;

class AbstractComplication
{
	Complication *complication;
	uint8_t type;

	template<typename T>
	AbstractComplication(T *ptr)
		: complication(ptr)
		, type(complication_type_map<T>)
	{}

public:
	AbstractComplication() : type(complication_type_map<void>) {}

	template<typename T>
	auto downcast() -> T* {
		if(type == complication_type_map<T>) {
			return static_cast<T*>(complication);
		}
		else {
			return nullptr;
		}
	}

	bool valid() { return type != complication_type_map<void>; }

	operator Complication&() {
		return *complication;
	}

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
		default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Asked for bad complication type %i", type);
			return {};
		}
	}

	void destroy() {
		switch(type) {
		case complication_type_map<BatteryComplication>:
			delete static_cast<BatteryComplication*>(complication);
			break;
		case complication_type_map<DateComplication>:
			delete static_cast<DateComplication*>(complication);
			break;
		case complication_type_map<WeatherComplication>:
			delete static_cast<WeatherComplication*>(complication);
			break;
		default:
			break;
		}
		type = complication_type_map<void>;
	}
};

#endif
