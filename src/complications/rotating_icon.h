#ifndef ROTATING_ICON_H
#define ROTATING_ICON_H

#include "animated.h"
#include "boulder.h"
#include "lazy_icon.h"

#include "pebble.h"

class RotatingIcon: private AnimatedCallback
{
	LazyIcon rotated_icon;

	using angle_t = int32_t;
	Animated<angle_t> angle;
	angle_t prev_angle;

public:
	RotatingIcon() = default;
	RotatingIcon(uint32_t resource_id, GRect container_bounds)
		: rotated_icon(resource_id, container_bounds)
	{
		angle.set_callback(this);
	}
	~RotatingIcon() = default;

	void reset(uint32_t new_id, GRect container_bounds) {
		if(rotated_icon.reset(new_id, container_bounds)) {
			prev_angle = 0;
			rerotate();
			set_angle(0);
		}
	}

	void set_angle(angle_t new_angle) {
		prev_angle = angle;
		angle = new_angle;
	}

private:
	void rerotate() {
		// Hopefully this does not introduce too much error over time
		rotated_icon.iterate([this](GDrawCommand *command, uint32_t) {
			auto cmd_type = gdraw_command_get_type(command);
			if(cmd_type == GDrawCommandTypePrecisePath ||
			   cmd_type == GDrawCommandTypePath) {
				auto num_points = gdraw_command_get_num_points(command);
				for(auto i = 0; i < num_points; ++i) {
					GPoint point = gdraw_command_get_point(command, i);
					auto point_angle = atan2_lookup(point.y, point.x);
					point_angle += this->angle - this->prev_angle;

					point.y = sin_lookup(point_angle);
					point.x = cos_lookup(point_angle);

					gdraw_command_set_point(command, i, point);
				}
			}
			return true;
		});
	}

	virtual void on_animated_update() override {
		rerotate();
		prev_angle = angle;
	}
};

#endif
