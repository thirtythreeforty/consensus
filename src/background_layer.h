#ifndef BACKGROUND_LAYER_H
#define BACKGROUND_LAYER_H

#include "boulder.h"

class BackgroundLayer : public Boulder::Layer
{
	Boulder::GDrawCommandImage ticks_image;

public:
	BackgroundLayer(GRect frame);
	~BackgroundLayer() = default;

	void recolor();

protected:
	virtual void update(GContext *ctx) override;
};

#endif
