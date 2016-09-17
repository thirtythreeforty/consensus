#include "manual_animation.h"

ManualAnimation::ManualAnimation(Animation* anim)
	: anim(anim)
{
	AnimationSetupImplementation impl = animation_get_implementation(anim)->setup;
	if(impl != nullptr) {
		impl(anim);
	}
}

ManualAnimation::~ManualAnimation()
{
	AnimationTeardownImplementation impl = animation_get_implementation(anim)->teardown;
	if(impl != nullptr) {
		impl(anim);
	}
}

void ManualAnimation::update(AnimationProgress progress)
{
	animation_get_implementation(anim)->update(anim, progress);
}
