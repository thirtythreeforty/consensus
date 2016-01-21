#include "scalable_path.h"

ScalablePath::ScalablePath(const GPathInfo *path_info)
{
	GPathInfo our_info = *path_info;

	const unsigned int points_size = sizeof(GPoint) * path_info->num_points;
	our_info.points = new GPoint[path_info->num_points];
	memcpy(our_info.points, path_info->points, points_size);

	path = gpath_create(&our_info);
	original_points = path_info->points;
}

ScalablePath::~ScalablePath()
{
	delete[] path->points;
	gpath_destroy(path);
}

void ScalablePath::scale(AnimationProgress dist_normalized)
{
	for(uint32_t i = 0; i < path->num_points; ++i) {
		path->points[i].x = original_points[i].x *
		                    dist_normalized / ANIMATION_NORMALIZED_MAX;
		path->points[i].y = original_points[i].y *
		                    dist_normalized / ANIMATION_NORMALIZED_MAX;
	}
}

