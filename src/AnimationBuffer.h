#ifndef ANIMATIONBUFFER_H
#define ANIMATIONBUFFER_H

extern "C" {
#include <pebble.h>
}

#include "boulder.h"

#include <vector>

namespace AnimationBuffer
{
	void enqueue(Animation*);
	void hold_all();
	void release_all();
};

#endif
