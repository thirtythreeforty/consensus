#ifndef COMPLICATION_H
#define COMPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pebble.h>

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

#ifdef __cplusplus
} // extern "C"

#include <experimental/optional>

#include "boulder.h"

class Complication: public Boulder::Layer
{
protected:
	explicit Complication(GRect frame) : Boulder::Layer(frame) {}
	~Complication() = default;
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

protected:
	void update(GContext*) override;

private:
	GRect calculate_date_frame();
	void set_displayed(uint8_t mday);

	static void spin_animation_started(Animation *animation, void *context);
	static void spin_animation_stopped(Animation *animation, bool finished, void *context);
	static void spin_animation_update(Animation* anim, AnimationProgress progress);
};

class WeatherComplication: public Complication
{
	struct WeatherAngles {
		int32_t temp_angle;
		int32_t humidity_angle;
	};

	WeatherAngles angles;
	std::experimental::optional<Boulder::GDrawCommandImage> icon;
	GPoint icon_shift;

	WeatherData requested_weather;
	bool animating;

	std::experimental::optional<Boulder::AppTimer> refresh_timer;

public:
	explicit WeatherComplication(GRect frame);
	~WeatherComplication() = default;

	void weather_changed(const WeatherData &new_weather);

private:
	void update(GContext *ctx) override;
	void animate_to(WeatherAngles angles);

	void schedule_refresh();

	static const WeatherAngles& get_angles(const WeatherComplication& complication);
	static void set_angles(WeatherComplication& complication, const WeatherAngles& angles);

	static WeatherAngles compute_angles(const WeatherData& wdata);
	static void request_refresh(void*);
	static void spinup_animation_started(Animation *anim, void *context);
	static void spinup_animation_stopped(Animation *anim, bool finished, void *context);

	friend WeatherAngles interpolate(uint32_t distance, WeatherAngles& from, WeatherAngles& to);
};

class BatteryComplication: public Complication
{
	bool animating;

	BatteryChargeState requested_state;

	uint32_t angle;
	std::experimental::optional<Boulder::GDrawCommandImage> icon;

public:
	explicit BatteryComplication(GRect frame);

	void state_changed(const BatteryChargeState *state);
	void update(GContext *ctx) override;

private:
	static void spinup_started(Animation *anim, void *context);
	static void spinup_stopped(Animation *anim, bool finished, void *context);

	static uint32_t angle_get(void *subject);
	static void angle_set(void *subject, uint32_t angle);
	void animate_to(uint32_t angle);
};

template<typename T> constexpr uint8_t complication_type_map;
template<> constexpr uint8_t complication_type_map<void> = 0;
template<> constexpr uint8_t complication_type_map<BatteryComplication> = 1;
template<> constexpr uint8_t complication_type_map<DateComplication> = 2;
template<> constexpr uint8_t complication_type_map<WeatherComplication> = 3;

class AbstractComplication
{
	void *complication;
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

	template<typename T>
	static auto from(T *ptr) -> AbstractComplication {
		return AbstractComplication{ptr};
	}

	void destroy() {
		switch(type) {
		case complication_type_map<void>:
			break;
		case complication_type_map<BatteryComplication>:
			delete static_cast<BatteryComplication*>(complication);
			break;
		case complication_type_map<DateComplication>:
			delete static_cast<DateComplication*>(complication);
			break;
		case complication_type_map<WeatherComplication>:
			delete static_cast<WeatherComplication*>(complication);
			break;
		}
		type = complication_type_map<void>;
	}
};
#endif

#endif
