#ifndef BOULDER_H
#define BOULDER_H

extern "C" {
#include "pebble.h"
}

#include <functional>
#include <memory>

#define B_PROXY_METHOD(MethodName, Prefix, WrappedObject) \
	template<class...Args> auto MethodName(Args&&... args) { \
		return Prefix ## MethodName(WrappedObject, std::forward<Args>(args)...); \
	}

namespace Boulder {

class Layer {
	::Layer* _layer;

public:
	explicit Layer(GRect frame)
		: _layer(layer_create_with_data(frame, sizeof(Layer*)))
	{
		*static_cast<Layer**>(layer_get_data(_layer)) = this;
		layer_set_update_proc(_layer, [](::Layer *layer, GContext *ctx){
			Layer* b_layer = *static_cast<Layer**>(layer_get_data(layer));
			b_layer->update(ctx);
		});
	}
	virtual ~Layer() {
		if(_layer) layer_destroy(_layer);
	}

	#define B_LAYER_METHOD(M) B_PROXY_METHOD(M, layer_, _layer)

	B_LAYER_METHOD(mark_dirty);
	B_LAYER_METHOD(get_frame);
	B_LAYER_METHOD(set_frame);
	B_LAYER_METHOD(get_bounds);
	B_LAYER_METHOD(set_bounds);
	B_LAYER_METHOD(convert_point_to_screen);
	B_LAYER_METHOD(convert_rect_to_screen);
	B_LAYER_METHOD(remove_from_parent);
	B_LAYER_METHOD(remove_child_layers);
	B_LAYER_METHOD(add_child);
	B_LAYER_METHOD(insert_below_sibling);
	B_LAYER_METHOD(insert_above_sibling);
	B_LAYER_METHOD(get_hidden);
	B_LAYER_METHOD(set_hidden);
	B_LAYER_METHOD(get_clips);
	B_LAYER_METHOD(set_clips);

	#undef B_LAYER_METHOD

	operator ::Layer*() {
		return _layer;
	}
protected:
	virtual void update(GContext *ctx) {}
};

class AppTimer {
	::AppTimer *timer;

public:
	AppTimer(uint32_t delay_ms, void (*callback)(void*), void *ptr)
		: timer(app_timer_register(delay_ms, callback, ptr))
	{}

	~AppTimer() {
		app_timer_cancel(timer);
	}

	void reschedule(uint32_t new_timeout_ms) {
		app_timer_reschedule(timer, new_timeout_ms);
	}
};

class GDrawCommandImage {
	::GDrawCommandImage* _image;

public:
	explicit GDrawCommandImage(uint32_t resource_id)
		: _image(gdraw_command_image_create_with_resource(resource_id))
	{}
	GDrawCommandImage(GDrawCommandImage& that)
		: _image(gdraw_command_image_clone(that._image))
	{}
	~GDrawCommandImage() {
		gdraw_command_image_destroy(_image);
	}

	void draw(::GContext *ctx, ::GPoint offset) const {
		gdraw_command_image_draw(ctx, _image, offset);
	}

	#define B_GDRAWCOMMANDIMAGE_METHOD(M) \
		B_PROXY_METHOD(M, gdraw_command_image_, _image)

	B_GDRAWCOMMANDIMAGE_METHOD(get_bounds_size);
	B_GDRAWCOMMANDIMAGE_METHOD(set_bounds_size);

	template<typename F>
	void iterate(F functor) {
		gdraw_command_list_iterate(
			gdraw_command_image_get_command_list(_image),
			[](GDrawCommand *command, uint32_t index, void *ctx) -> bool {
				return (*static_cast<F*>(ctx))(command, index);
			},
			&functor
		);
	}
};

template<typename S, typename T>
using PropertyAnimationSetter = void (*)(S&, const T&);

template<typename S, typename T>
using PropertyAnimationGetter = const T& (*)(const S&);

template<typename S, typename T, PropertyAnimationSetter<S, T> Setter, PropertyAnimationGetter<S, T> Getter>
class PropertyAnimation {
	::PropertyAnimation *anim;

public:
	PropertyAnimation(S& subject, const T* from, const T* to) {
		static const PropertyAnimationImplementation impl = {
			.base = {
				NULL,
				(AnimationUpdateImplementation)PropertyAnimation::update,
				NULL,
			},
			.accessors = {
				{ (Int16Setter)Setter },
				{ (Int16Getter)Getter }
			}
		};
		anim = property_animation_create(&impl, &subject, NULL, NULL);
		if(from == nullptr) {
			from = &Getter(subject);
		}
		if(to == nullptr) {
			to = &Getter(subject);
		}
		property_animation_from(anim, const_cast<T*>(from), sizeof(T), true);
		property_animation_to(anim, const_cast<T*>(to), sizeof(T), true);
	}
	~PropertyAnimation() {
		if(!animation_is_scheduled(*this)) {
			property_animation_destroy(anim);
		}
	}

	void schedule() {
		animation_schedule(*this);
	}

	operator ::Animation*() {
		return reinterpret_cast<::Animation*>(anim);
	}

	operator ::PropertyAnimation*() {
		return anim;
	}

private:
	static void update(::PropertyAnimation *panim, const uint32_t distance) {
		T from, to;
		property_animation_from(panim, &from, sizeof(T), false);
		property_animation_to(panim, &to, sizeof(T), false);

		T interpolated = interpolate(distance, from, to);

		auto impl = reinterpret_cast<const PropertyAnimationImplementation*>(animation_get_implementation(reinterpret_cast<::Animation*>(panim)));
		auto setter = reinterpret_cast<PropertyAnimationSetter<S, T>>(impl->accessors.setter.int16);

		S *subject;
		property_animation_subject(panim, reinterpret_cast<void**>(&subject), false);

		if(setter && subject) {
			setter(*subject, interpolated);
		}
	}
};

}

#endif
