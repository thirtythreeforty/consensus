#ifndef ANIMATED_H
#define ANIMATED_H

#include "boulder.h"
#include "variant.h"

#include <utility>

namespace animated_detail {

template<typename T>
using CloseChecker = bool (*)(const T&, const T&);

class AnimatedCallback
{
	template<typename T, CloseChecker<T>>
	friend class Animated;
private:
	virtual void on_animated_update() = 0;
};

class DefaultAnimatedCallback: public AnimatedCallback {
private:
	virtual void on_animated_update() override {}
};
static DefaultAnimatedCallback default_callback;

template<typename T>
bool default_are_close(const T&, const T&) { return false; }

template<typename T, CloseChecker<T> TsClose>
class Animated
{
	// Currently there is no need to store requested in an Optional or something
	// similar because I'm just animating POD types like ints.
	static_assert(std::is_pod<T>::value, "Animated type is not POD");
	T requested;
	T current;
	bool animating;

	AnimatedCallback* callback;

public:
	Animated()
		: callback{&default_callback}
	{}

	void set_callback(AnimatedCallback* new_callback) {
		callback = new_callback;
	}

	void set(const T& new_t) {
		if(requested == new_t) {
			return;
		}

		requested = new_t;

		if(!animating) {
			animate_to_requested();
		}
	}

	const T& operator =(const T& other) {
		set(other);
		return requested;
	}

	operator const T&() {
		return current;
	}

private:
	void animate_to_requested() {
		if(TsClose(current, requested)) {
			current = requested;
			callback->on_animated_update();
			return;
		}

		animating = true;

		Boulder::PropertyAnimation<
			Animated<T, TsClose>, T,
			Animated<T, TsClose>::set_st, Animated<T, TsClose>::get_st
		> anim(*this, &current, &requested);

		static const AnimationHandlers handlers = {
			.started = NULL,
			.stopped = anim_stopped_handler,
		};

		static const unsigned int duration = 700;
		static const unsigned int delay = 200;

		animation_set_duration(anim, duration);
		animation_set_delay(anim, delay);
		animation_set_curve(anim, AnimationCurveEaseInOut);
		animation_set_handlers(anim, handlers, this);

		anim.schedule();
	}

	static void set_st(Animated<T, TsClose>& animated, const T& new_t) {
		animated.current = new_t;
		animated.callback->on_animated_update();
	}

	static const T& get_st(const Animated<T, TsClose>& animated) {
		return animated.current;
	}

	static void anim_stopped_handler(Animation *anim, bool finished, void* context) {
		auto animated = static_cast<Animated<T, TsClose>*>(context);

		animated->animating = false;

		if(!(animated->requested == animated->current)) {
			animated->animate_to_requested();
		}
	}
};

}

using animated_detail::AnimatedCallback;

template<typename T, animated_detail::CloseChecker<T> TsClose = animated_detail::default_are_close>
using Animated = animated_detail::Animated<T, TsClose>;

#endif
