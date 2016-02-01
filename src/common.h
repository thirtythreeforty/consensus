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

#include <pebble.h>
namespace {
	static time_t start_time;
	static uint16_t start_ms;
}
inline void debug_profile_start()
{
	time_ms(&start_time, &start_ms);
}
inline void debug_profile_end()
{
	time_t end_time;
	uint16_t end_ms;
	time_ms(&end_time, &end_ms);
	APP_DEBUG("Started @ %lu.%i, now %lu.%i (%lu ms elapsed)",
	          start_time, start_ms, end_time, end_ms,
	          (end_time - start_time) * 1000 + end_ms - start_ms);
}

#else
#define APP_DEBUG(...)
inline void debug_profile_start() {}
inline void debug_profile_end() {}
#endif

#endif
