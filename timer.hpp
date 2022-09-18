
#pragma once

#include <limits>
#include <stdexcept>

#include "timer_api.hpp"

class Timer
{
private:
	static inline bool timer_availabilities[2] = {true, true};

	int id;
	unsigned last_loaded = 0;
	
	void init() { timer_init(id); }
	void restore() { timer_restore(id); }
	void load(unsigned value) { timer_load(id, value); }
	unsigned read() { return timer_read(id); }
	
	explicit Timer(int id) : id{id} { init(); }

public:
	static constexpr unsigned int ticks_per_ms = 32;

	static Timer make_timer()
	{
		for (int i = 0; i < 2; i++)
		{
			if (timer_availabilities[i])
			{
				timer_availabilities[i] = false;
				return Timer{i};
			}
		}
		throw std::runtime_error("No more timers available");
	}

	~Timer()
	{
		restore();
		timer_availabilities[id] = true;
	}

	void set_raw(unsigned value) { load(value); }
	void set_ms(unsigned ms) { load(ms * ticks_per_ms); }

	unsigned get_raw() { return read(); }
	unsigned get_ms() { return read() / ticks_per_ms; }

};


class Stopwatch
{
private:
	Timer timer;

public:
	Stopwatch() : timer{Timer::make_timer()} {};

	void start() { timer.set_raw(std::numeric_limits<uint32_t>::max()); }
	double get_ms() { return (double)(std::numeric_limits<uint32_t>::max() - timer.get_raw()) / Timer::ticks_per_ms; }
};
