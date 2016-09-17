#ifndef ANIMATED_H
#define ANIMATED_H

#include "AnimationBuffer.h"
#include "boulder.h"
#include "preferences.h"
#include "variant.h"

#include <functional>
#include <utility>

namespace animated_detail {

template<typename T>
using CloseChecker = bool(const T&, const T&);

template<typename T>
using Clamp = T(const T&);

class AnimatedCallback
{
	template<typename T, CloseChecker<T> TsCheck, Clamp<T> TsClamp>
	friend class Animated;

private:
	virtual void on_animated_update(void *animated) = 0;
};

class DefaultAnimatedCallback: public AnimatedCallback {
private:
	virtual void on_animated_update(void*) override {}
};
static DefaultAnimatedCallback default_callback;

template<typename T>
bool default_are_close(const T& a, const T& b) { return a == b; }

template<typename T>
T default_clamp(const T& t) { return t; }

template<typename T, CloseChecker<T> TsClose, Clamp<T> TsClamp>
class Animated
{
	using Self = Animated<T, TsClose, TsClamp>;

	// Currently there is no need to store requested in an Optional or something
	// similar because I'm just animating POD types like ints.
	static_assert(std::is_pod<T>::value, "Animated type is not POD");
	T requested;
	T current;
	bool animating;

	std::reference_wrapper<AnimatedCallback> callback;

	unsigned int duration = 700;

public:
	Animated()
		: requested()
		, current()
		, animating(false)
		, callback(default_callback)
	{}

	Animated(const T& t, unsigned int duration)
		: requested(t)
		, current(t)
		, animating(false)
		, callback(default_callback)
		, duration(duration)
	{}

	Animated(const T& t)
		: Animated(t, 700)
	{}

	void set_callback(AnimatedCallback& new_callback) {
		callback = new_callback;
	}

	void set(const T& new_t) {
		if(requested == new_t) {
			return;
		}

		requested = TsClamp(new_t);

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
		if(TsClose(current, requested) || !should_animate()) {
			current = requested;
			callback.get().on_animated_update(this);
			return;
		}

		animating = true;

		Boulder::PropertyAnimation<
			Self, T,
			Self::set_st, Self::get_st
		> anim(*this, &current, &requested);

		static const AnimationHandlers handlers = {
			.started = NULL,
			.stopped = anim_stopped_handler,
		};

		animation_set_duration(anim, duration);
		animation_set_curve(anim, AnimationCurveEaseInOut);
		animation_set_handlers(anim, handlers, this);

		AnimationBuffer::enqueue(anim.release());
	}

	static void set_st(Self* animated, T new_t) {
		animated->current = new_t;
		animated->callback.get().on_animated_update(animated);
	}

	static T get_st(const Self* animated) {
		return animated->current;
	}

	static void anim_stopped_handler(Animation *anim, bool finished, void* context) {
		auto animated = static_cast<Self*>(context);

		animated->animating = false;

		if(!(animated->requested == animated->current)) {
			animated->animate_to_requested();
		}
	}
};

}

using animated_detail::AnimatedCallback;

template<typename T, animated_detail::CloseChecker<T> TsClose = animated_detail::default_are_close, animated_detail::Clamp<T> TsClamp = animated_detail::default_clamp>
using Animated = animated_detail::Animated<T, TsClose, TsClamp>;

#endif
