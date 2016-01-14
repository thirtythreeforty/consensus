#ifndef COMPLICATION_COMMON_H
#define COMPLICATION_COMMON_H

#include "complication.h"

#ifdef __cplusplus
extern "C" {
#endif

void base_complication_update_1(Layer *layer, GContext *ctx,
                                GColor color, int32_t max_angle);
void base_complication_update_2(Layer *layer, GContext *ctx,
                                GColor color_1, int32_t max_angle_1,
                                GColor color_2, int32_t max_angle_2);
Animation* base_complication_animate_in(const AnimationImplementation *impl,
                                        const AnimationHandlers *handlers, void *ctx);

Animation* base_complication_setup_animation(Animation *anim,
                                       const AnimationHandlers *handlers, void *ctx);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
