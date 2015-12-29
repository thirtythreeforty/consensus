#include "complication.h"

void base_complication_update_1(Layer *layer, GContext *ctx,
                                GColor color, int32_t max_angle);
void base_complication_update_2(Layer *layer, GContext *ctx,
                                GColor color_1, int32_t max_angle_1,
                                GColor color_2, int32_t max_angle_2);
Animation* base_complication_animate_in(const AnimationImplementation *impl,
                                        const AnimationHandlers *handlers, void *ctx);
