#ifndef LAZY_ICON_H
#define LAZY_ICON_H

#include "boulder.h"

class LazyIcon
{
	Boulder::GDrawCommandImage image;
	uint32_t current_id;

	static constexpr uint32_t none_id = -1; // hopefully we won't have 4 billion resources

public:
	LazyIcon()
		: image()
		, current_id(none_id)
	{}
	LazyIcon(uint32_t resource_id)
		: image(resource_id)
		, current_id(resource_id)
	{}
	~LazyIcon() = default;

	void reset(uint32_t new_id) {
		if(current_id != new_id) {
			image = Boulder::GDrawCommandImage(new_id);
			current_id = new_id;
		}
	}

	void reset() {
		if(current_id != none_id) {
			image = Boulder::GDrawCommandImage();
			current_id = none_id;
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
};

#endif
