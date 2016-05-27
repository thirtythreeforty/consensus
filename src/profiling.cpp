extern "C" {
#include <pebble.h>
}

#include <algorithm>
#include <map>
#include <vector>

namespace {

struct TimeSpent {
	int32_t exclusive_time;
	int32_t total_time;

	TimeSpent() : exclusive_time(0), total_time(0) {}
};

std::map<void*, TimeSpent> times;
std::vector<std::pair<void*, int32_t>> last_entry_time;

}

void setup_profiling()
{
	last_entry_time.reserve(100);
}

int64_t to_ms(time_t t, uint16_t ms)
{
	int64_t r = t;
	r *= 1000;
	r += ms;

	if(r < 0)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "time < 0, cannot happen");
	}

	return r;
}

extern "C" void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	time_t t; uint16_t ms;
	time_ms(&t, &ms);
	auto r = to_ms(t, ms);

	last_entry_time.emplace_back(this_fn, r);
}

extern "C" void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	time_t t;
	uint16_t ms;
	time_ms(&t, &ms);

	auto now = to_ms(t, ms);

	auto entered_at = last_entry_time.back().second;
	last_entry_time.pop_back();
	times[this_fn].exclusive_time += (now - entered_at);
	times[this_fn].total_time += (now - entered_at);

	if(!last_entry_time.empty())
		times[last_entry_time.back().first].exclusive_time -= (now - entered_at);
}

void show_instrumentation()
{
	std::vector<std::pair<void*, TimeSpent>> sorted_times;
	sorted_times.resize(20);
	std::partial_sort_copy(times.begin(), times.end(), sorted_times.begin(), sorted_times.end(),
	                       [](auto a, auto b) { return a.second.exclusive_time > b.second.exclusive_time; });

	for(auto& i: sorted_times) {
		APP_LOG(APP_LOG_LEVEL_INFO, "%p : %li ms, %li ms", i.first, i.second.total_time, i.second.exclusive_time);
	}

	void init();
	void ignore_connection_change(bool);
	APP_LOG(APP_LOG_LEVEL_INFO, "init (%p) spent %li ms total and %li ms exclusive", (void*)init, times[(void*)init].total_time, times[(void*)init].exclusive_time);
	APP_LOG(APP_LOG_LEVEL_INFO, "ignore_connection_change (%p) spent %li ms total and %li ms exclusive", (void*)ignore_connection_change, times[(void*)ignore_connection_change].total_time, times[(void*)ignore_connection_change].exclusive_time);

	times.clear();
}
