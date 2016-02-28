#include "background_layer.h"

#include "themes.h"

BackgroundLayer::BackgroundLayer(GRect frame)
	: Layer(frame)
	, ticks_image(RESOURCE_ID_TICKS)
{
	recolor();
}

void BackgroundLayer::recolor()
{
	ticks_image.recolor(theme().tick_color, GColorClear);
}

void BackgroundLayer::update(GContext *ctx)
{
	graphics_context_set_fill_color(ctx, theme().background_color);
	graphics_fill_rect(ctx, get_bounds(), 0, GCornerNone);

	ticks_image.draw(ctx, GPointZero);
}
