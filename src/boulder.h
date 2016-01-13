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

	static void update_stub(::Layer *layer, GContext *ctx) {
		Layer* b_layer = *static_cast<Layer**>(layer_get_data(layer));
		b_layer->update(ctx);
	}

	operator ::Layer*() {
		return _layer;
	}

public:
	explicit Layer(GRect frame)
		: _layer(layer_create_with_data(frame, sizeof(Layer*)))
	{
		layer_set_update_proc(_layer, Layer::update_stub);
		*static_cast<Layer**>(layer_get_data(_layer)) = this;
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
protected:
	virtual void update(GContext *ctx) {}
};

}

#endif
