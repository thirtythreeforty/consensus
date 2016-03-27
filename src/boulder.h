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

#define B_CHECKED_PROXY_METHOD(MethodName, Prefix, WrappedObject, DefaultValue) \
	template<class...Args> auto MethodName(Args&&... args) { \
		if(WrappedObject != nullptr) { \
			return Prefix ## MethodName(WrappedObject, std::forward<Args>(args)...); \
		} \
		else { \
			return DefaultValue; \
		} \
	}
#define VOID_RETURN ((void)0)

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

class Window {
	::Window* _window;
public:
	explicit Window()
		: _window(window_create())
	{
		window_set_user_data(_window, this);
		window_set_window_handlers(_window, _handlers);
		// Not enabling this yet, because I don't need it and I don't know how
		// this system works exactly
		//window_set_click_config_provider_with_context(_window, on_click_st, this);
	}

	virtual ~Window() {
		window_destroy(_window);
	}

	GRect get_bounds() {
		return layer_get_bounds(window_get_root_layer(_window));
	}

	operator ::Window*() {
		return _window;
	}

protected:
	virtual void on_load() {}
	virtual void on_appear() {}
	virtual void on_disappear() {}
	virtual void on_unload() {}
	// virtual void on_click() {}

	void add_child(::Layer *layer) {
		layer_add_child(window_get_root_layer(_window), layer);
	}

private:
#define B_WINDOW_CALLBACK(CallbackName, MemberName) \
	static void CallbackName(::Window *w) { \
		Window* bwin = static_cast<Window*>(window_get_user_data(w)); \
		bwin->MemberName(); \
	}

	B_WINDOW_CALLBACK(on_load_st, on_load);
	B_WINDOW_CALLBACK(on_appear_st, on_appear);
	B_WINDOW_CALLBACK(on_disappear_st, on_disappear);
	B_WINDOW_CALLBACK(on_unload_st, on_unload);

	// static void on_click_st(void *context) {
	// 	Window* bwin = static_cast<Window*>(context);
	// 	bwin->on_click();
	// }

#undef B_WINDOW_CALLBACK

	static const WindowHandlers _handlers;
};

namespace WindowStack {
	inline void push(Window& window, bool animated) {
		window_stack_push(window, animated);
	}
	inline void pop(bool animated) {
		window_stack_pop(animated);
	}
	inline void pop_all(bool animated) {
		window_stack_pop_all(animated);
	}
	inline void remove(Window& window, bool animated) {
		window_stack_remove(window, animated);
	}
	inline Window& top_window() {
		auto w = window_stack_get_top_window();
		return *static_cast<Window*>(window_get_user_data(w));
	}
	inline bool contains(Window& window) {
		return window_stack_contains_window(window);
	}
}

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

namespace AppTimer {
	template<typename T>
	class TimerHandle {
		::AppTimer* handle;
		T* func;

	public:
		TimerHandle(::AppTimer *handle, T* func)
			: handle(handle)
			, func(func)
		{}

		void cancel() {
			app_timer_cancel(handle);
			delete func;
		}

		void reschedule(uint32_t new_timeout_ms) {
			app_timer_reschedule(handle, new_timeout_ms);
		}
	};

	namespace {
		template<typename F>
		void on_timer(void* f) {
			auto func = static_cast<F*>(f);
			(*func)();
			delete func;
		}
	}

	template<typename T>
	TimerHandle<T> create(uint32_t delay_ms, T f) {
		auto func = new T(std::move(f));
		auto handle = app_timer_register(delay_ms, on_timer<T>, func);
		return {handle, func};
	}
};

class GDrawCommandImage {
	::GDrawCommandImage* _image;

public:
	GDrawCommandImage()
		: _image(nullptr)
	{}
	explicit GDrawCommandImage(uint32_t resource_id)
		: _image(gdraw_command_image_create_with_resource(resource_id))
	{}
	GDrawCommandImage(GDrawCommandImage& that)
		: _image(gdraw_command_image_clone(that._image))
	{}
	GDrawCommandImage(GDrawCommandImage&& that)
		: _image(that._image)
	{
		that._image = nullptr;
	}
	GDrawCommandImage& operator=(GDrawCommandImage that) {
		swap(that);
		return *this;
	}
	~GDrawCommandImage() {
		if(_image) {
			gdraw_command_image_destroy(_image);
		}
	}
	void swap(GDrawCommandImage& that) {
		std::swap(this->_image, that._image);
	}

	void draw(::GContext *ctx, ::GPoint offset) const {
		if(_image != nullptr) {
			gdraw_command_image_draw(ctx, _image, offset);
		}
	}

	#define B_GDRAWCOMMANDIMAGE_METHOD(M, DefaultValue) \
		B_CHECKED_PROXY_METHOD(M, gdraw_command_image_, _image, DefaultValue)

	B_GDRAWCOMMANDIMAGE_METHOD(get_bounds_size, GSizeZero);
	B_GDRAWCOMMANDIMAGE_METHOD(set_bounds_size, VOID_RETURN);

	template<typename F>
	void iterate(F functor) {
		if(_image == nullptr) {
			return;
		}
		gdraw_command_list_iterate(
			gdraw_command_image_get_command_list(_image),
			[](GDrawCommand *command, uint32_t index, void *ctx) -> bool {
				return (*static_cast<F*>(ctx))(command, index);
			},
			&functor
		);
	}

	void recolor(GColor stroke, GColor fill) {
		iterate([=](GDrawCommand *command, uint32_t) {
			gdraw_command_set_fill_color(command, fill);
			gdraw_command_set_stroke_color(command, stroke);
			return true;
		});
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

	// Provide a default interpolate() when then template will instantiate.
	// Also, Pebble's uint32 interpolator is very, very broken.  But the user
	// will expect it to be defined.  This takes care of that, too.
	template<typename T>
	inline auto interpolate(uint32_t distance, const T& from, const T& to)
		-> std::enable_if_t<std::is_unsigned<T>::value, T>
	{
		if(to > from)
			return from + (((uint64_t)distance * (to - from)) / ANIMATION_NORMALIZED_MAX);
		else
			return from - (((uint64_t)distance * (from - to)) / ANIMATION_NORMALIZED_MAX);
	}

	template<typename T>
	inline auto interpolate(uint32_t distance, const T& from, const T& to)
		-> std::enable_if_t<std::is_signed<T>::value, T>
	{
		return from + (((int64_t)distance * (to - from)) / ANIMATION_NORMALIZED_MAX);
	}

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

class Tuple {
	::Tuple *_tuple;
public:
	Tuple(::Tuple *tuple)
		: _tuple(tuple)
	{}
	::TupleType type() const {
		return _tuple->type;
	}
	template<typename T> inline T value() const;

	bool valid() const { return _tuple != nullptr; }
};

template<>
inline uint8_t Tuple::value<uint8_t>() const {
	return _tuple->value[0].uint8;
}
template<>
inline uint16_t Tuple::value<uint16_t>() const {
	return _tuple->value[0].uint16;
}
template<>
inline uint32_t Tuple::value<uint32_t>() const {
	return _tuple->value[0].uint32;
}
template<>
inline unsigned int Tuple::value<unsigned int>() const {
	return _tuple->value[0].uint32;
}
template<>
inline bool Tuple::value<bool>() const {
	return _tuple->value[0].uint32;
}

namespace persist
{

// These templates will not be defined, except for their specializations
inline bool exists(uint32_t persist_key)
{
	return persist_exists(persist_key);
}

inline void remove(uint32_t persist_key)
{
	persist_delete(persist_key);
}

template<typename T>
inline void save(uint32_t persist_key, T value);
template<typename T>
inline T load(uint32_t persist_key);
template<typename T>
inline T load_default(uint32_t persist_key, T default_value);

#define B_SPECIALIZE_PERSIST(Type, FunctionSuffix) \
	template<> \
	inline void save<Type>(uint32_t persist_key, Type value) { \
		persist_write_ ## FunctionSuffix (persist_key, value); \
	} \
	template<> \
	inline Type load<Type>(uint32_t persist_key) { \
		return persist_read_ ## FunctionSuffix (persist_key); \
	} \
	template<> \
	inline Type load_default<Type>(uint32_t persist_key, Type default_value) { \
		if(persist_exists(persist_key)) \
			return persist_read_ ## FunctionSuffix (persist_key); \
		else \
			return default_value; \
	}

B_SPECIALIZE_PERSIST(bool, bool)
B_SPECIALIZE_PERSIST(unsigned int, int)
B_SPECIALIZE_PERSIST(uint32_t, int)
B_SPECIALIZE_PERSIST(uint8_t, int)

template<typename T>
inline void save_data(uint32_t persist_key, const T& value)
{
	static_assert(sizeof(T) < 256, "Type too large to store!");
	persist_write_data(persist_key, &value, sizeof(T));
}

template<typename T>
inline void load_data(uint32_t persist_key, T& value)
{
	static_assert(sizeof(T) < 256, "Type too large to store!");
	persist_read_data(persist_key, &value, sizeof(T));
}

}

}

#endif
