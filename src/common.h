#ifndef COMMON_H
#define COMMON_H

#include <algorithm>

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

template<typename T, typename U, typename V>
inline const T& clamp(const T& min, const U& x, const V& max)
{
	return std::max(min, std::min(x, max));
}

#define DEBUG

#ifdef DEBUG
#define APP_DEBUG(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define APP_DEBUG(...)
#endif

#endif
