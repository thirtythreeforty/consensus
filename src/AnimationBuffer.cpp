#include "AnimationBuffer.h"

namespace AnimationBuffer {

std::vector<Animation*> animations;
bool hold;

void enqueue(Animation *anim)
{
	if(hold) {
		animations.emplace_back(anim);

		auto setup = animation_get_implementation(anim)->setup;
		if(setup) {
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Calling setup function");
			setup(anim);
		}
	}
	else {
		animation_schedule(anim);
	}
}

void hold_all()
{
	hold = true;
}

void release_all()
{
	hold = false;
	for(auto& anim: animations) {
		animation_schedule(anim);
	}
	animations.clear();
}

}
