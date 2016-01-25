#ifndef SCALABLE_PATH_H
#define SCALABLE_PATH_H

extern "C" {
#include <pebble.h>
}

class ScalablePath
{
	GPoint *original_points;
	GPath *path;

public:
	ScalablePath(const GPathInfo *path_info);
	~ScalablePath();
	void scale(AnimationProgress dist_normalized);

	operator GPath*() {
		return path;
	}
};


#endif
