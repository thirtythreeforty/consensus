#ifndef SCALABLE_PATH_H
#define SCALABLE_PATH_H

#include <pebble.h>

struct ScalablePath;
typedef struct ScalablePath ScalablePath;

ScalablePath* scalable_path_create(const GPathInfo *path_info);
void scalable_path_destroy(ScalablePath *spath);
void scalable_path_scale(ScalablePath *spath, AnimationProgress dist_normalized);
GPath* scalable_path_get_path(const ScalablePath *spath);

#endif
