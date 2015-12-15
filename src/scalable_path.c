#include "scalable_path.h"

struct ScalablePath {
	GPoint *original_points;
	GPath *path;
};

ScalablePath* create_scalable_path(const GPathInfo *path_info)
{
	GPathInfo our_info = *path_info;

	const unsigned int points_size = sizeof(GPoint) * path_info->num_points;
	our_info.points = malloc(points_size);
	memcpy(our_info.points, path_info->points, points_size);

	ScalablePath *spath = malloc(sizeof(ScalablePath));
	spath->path = gpath_create(&our_info);
	spath->original_points = path_info->points;
	return spath;
}

void destroy_scalable_path(ScalablePath *spath)
{
	free(spath->path->points);
	gpath_destroy(spath->path);
	free(spath);
}

void scalable_path_scale(ScalablePath *spath, AnimationProgress dist_normalized)
{
	for(uint32_t i = 0; i < spath->path->num_points; ++i) {
		spath->path->points[i].x = spath->original_points[i].x *
		                            dist_normalized / ANIMATION_NORMALIZED_MAX;
		spath->path->points[i].y = spath->original_points[i].y *
		                            dist_normalized / ANIMATION_NORMALIZED_MAX;
	}
}

GPath* scalable_path_get_path(const ScalablePath *spath)
{
	return spath->path;
}

