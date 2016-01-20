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
	Layer(Layer&) = delete;

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

class TextLayer
{
	::TextLayer *text_layer;

public:
	TextLayer(GRect frame)
		: text_layer(text_layer_create(frame))
	{}
	TextLayer(TextLayer&) = delete;

	~TextLayer() {
		text_layer_destroy(text_layer);
	}

	#define B_TEXTLAYER_METHOD(M) B_PROXY_METHOD(M, text_layer_, text_layer)

	B_TEXTLAYER_METHOD(set_text);
	B_TEXTLAYER_METHOD(get_text);
	B_TEXTLAYER_METHOD(set_background_color);
	B_TEXTLAYER_METHOD(set_text_color);
	B_TEXTLAYER_METHOD(set_overflow_mode);
	B_TEXTLAYER_METHOD(set_font);
	B_TEXTLAYER_METHOD(set_text_alignment);
	B_TEXTLAYER_METHOD(enable_screen_text_flow_and_paging);
	B_TEXTLAYER_METHOD(restore_default_text_flow_and_paging);
	B_TEXTLAYER_METHOD(get_content_size);
	B_TEXTLAYER_METHOD(set_size);

	#undef B_TEXTLAYER_METHOD

	operator ::TextLayer*() {
		return text_layer;
	}

	operator ::Layer*() {
		return text_layer_get_layer(text_layer);
	}
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


/*  The PropertyAnimation class provides a type-safe abstraction over the
 *  Pebble SDK's type-erased version.  It handles all the update function
 *  boilerplate needed for any type being animated.  Users only need to define
 *  a suitable interpolation function for the type T:
 *      T interpolate(uint32_t distance, T& from, T& to);
 *  This implementation also utilizes Pebble's built-in update methods for all
 *  types they provide implementations for, so using this wrapper with such a
 *  type incurs no code size overhead and does not require the definition of an
 *  interpolate() function.
 */
namespace {
	// The code in this namespace{} is helper code for the PropertyAnimation class.

	template<typename S, typename T>
	using PropertyAnimationSetter = void (*)(S&, const T&);

	template<typename S, typename T>
	using PropertyAnimationGetter = const T& (*)(const S&);

	using updater_t = void (*)(::PropertyAnimation*, const uint32_t);

	template<typename S, typename T, PropertyAnimationSetter<S, T> Setter>
	struct PAUpdate {
		/* This struct is particularly interesting.  It provides the default
		 * templated implementation of the update function (templates being
		 * necessary to ensure type-safety, exactly as if the user were
		 * specifying it).  Such code is needed for every type T being animated
		 * and is quite boring -- the only interesting parts will happen with
		 * the actual interpolation and storing the calculated value.  The user
		 * supplies both interesting functions.  This struct also provides
		 * specializations for Pebble's built-in types, which allows us to
		 * select their implementation for free at compile time, exactly as if
		 * the user were manually specifying the
		 * PropertyAnimationImplementation.
		 */

		static void update(::PropertyAnimation *panim, const uint32_t distance) {
			T from, to;
			property_animation_from(panim, &from, sizeof(T), false);
			property_animation_to(panim, &to, sizeof(T), false);

			T interpolated = interpolate(distance, from, to);

			auto anim = reinterpret_cast<::Animation*>(panim);
			auto impl = reinterpret_cast<const ::PropertyAnimationImplementation*>(animation_get_implementation(anim));
			auto setter = reinterpret_cast<PropertyAnimationSetter<S, T>>(impl->accessors.setter.int16);

			S *subject;
			property_animation_subject(panim, reinterpret_cast<void**>(&subject), false);

			// subject cannot be null, because it was a reference in the PropertyAnimation ctor
			// setter cannot be null, because it was statically checked in the ctor
			setter(*subject, interpolated);
		}
	};

#define B_SPECIALIZE_PAUPDATE(Type, BuiltInFunc) \
	template<typename S, PropertyAnimationSetter<S, Type> Setter> \
	struct PAUpdate<S, Type, Setter> { \
		static constexpr updater_t update = BuiltInFunc; \
	};

	B_SPECIALIZE_PAUPDATE(int16_t, property_animation_update_int16);
	B_SPECIALIZE_PAUPDATE(uint32_t, property_animation_update_uint32);
	B_SPECIALIZE_PAUPDATE(::GPoint, property_animation_update_gpoint);
	B_SPECIALIZE_PAUPDATE(::GRect, property_animation_update_grect);
	B_SPECIALIZE_PAUPDATE(::GColor8, property_animation_update_gcolor8);

#undef B_SPECIALIZE_PAUPDATE

}

template<typename S, typename T, PropertyAnimationSetter<S, T> Setter, PropertyAnimationGetter<S, T> Getter>
class PropertyAnimation {
	::PropertyAnimation *anim;

public:
	PropertyAnimation(S& subject, const T* from, const T* to) {
		static_assert(Setter != nullptr, "Setter cannot be null");
		static_assert(Getter != nullptr, "Getter cannot be null");

		static const PropertyAnimationImplementation impl = {
			.base = {
				NULL,
				(AnimationUpdateImplementation)PAUpdate<S, T, Setter>::update,
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
};

}

#endif
