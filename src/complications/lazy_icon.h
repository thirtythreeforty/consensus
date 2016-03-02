#ifndef LAZY_ICON_H
#define LAZY_ICON_H

extern "C" {
#include "pebble.h"
}

#include "boulder.h"
#include "themes.h"

class LazyIcon
{
	Boulder::GDrawCommandImage image;
	uint32_t current_id;
	GPoint icon_shift;

	static constexpr uint32_t none_id = -1; // hopefully we won't have 4 billion resources

public:
	LazyIcon()
		: current_id(none_id)
		, icon_shift(GPointZero)
	{}
	LazyIcon(uint32_t resource_id, GRect container_bounds)
		: image(resource_id)
		, current_id(resource_id)
		, icon_shift(compute_shift(container_bounds))
	{}
	~LazyIcon() = default;

	void reset(uint32_t new_id, GRect container_bounds) {
		if(current_id != new_id) {
			image = Boulder::GDrawCommandImage(new_id);
			current_id = new_id;
		}
		align(container_bounds);
		recolor();
	}

	void reset() {
		if(current_id != none_id) {
			image = Boulder::GDrawCommandImage();
			current_id = none_id;
		}
	}

	void align(GRect container_bounds) {
		icon_shift = compute_shift(container_bounds);
	}

	void recolor() {
		if(current_id != none_id) {
			image.recolor(theme().complication_icon_color(), GColorClear);
		}
	}

	void draw(GContext *ctx) {
		if(current_id != none_id) {
			image.draw(ctx, icon_shift);
		}
	}

#define LAZY_PROXY_METHOD(MethodName, DefaultValue) \
	template<class...Args> auto MethodName(Args&&... args) { \
		if(current_id != none_id) \
			return image.MethodName(std::forward<Args>(args)...); \
		else \
			return DefaultValue; \
	}

	LAZY_PROXY_METHOD(draw,);
	LAZY_PROXY_METHOD(iterate,);
	LAZY_PROXY_METHOD(get_bounds_size, GSizeZero);
	LAZY_PROXY_METHOD(set_bounds_size,);

private:
	GPoint compute_shift(GRect container_bounds) {
		if(current_id != none_id) {
			const GSize icon_size = image.get_bounds_size();
			return (GPoint){
				.x = static_cast<int16_t>(container_bounds.size.w / 2 - icon_size.w / 2),
				.y = static_cast<int16_t>(container_bounds.size.h / 2 - icon_size.h / 2)
			};
		}
		else return GPointZero;
	}
};

#endif
