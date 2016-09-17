#ifndef MANUAL_ANIMATION_H
#define MANUAL_ANIMATION_H

extern "C" {
#include <pebble.h>
}

class ManualAnimation
{
	Animation* anim;

public:
	ManualAnimation(Animation* anim);
	~ManualAnimation();

	void update(AnimationProgress progress);
};

#endif
