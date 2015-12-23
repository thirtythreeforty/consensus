#include "complication.h"

void base_complication_update(Layer *layer, GContext *ctx,
                              GColor color, int32_t max_angle);
void base_complication_animate_in(const AnimationImplementation *impl,
                                  const AnimationHandlers *handlers, void *ctx);
