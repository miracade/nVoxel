
#include "timer.hpp"

#include <cstdint>

constexpr std::uintptr_t timer_base_addr = 0x900D0000;
unsigned timer_ctl_bkp[2], timer_load_bkp[2];



void timer_init(unsigned timer)
{
	/* Timer configuration :
		- timers not freezed
		- timers decreasing
		- timers count to 0 and stop until a new value is loaded
	*/
	volatile unsigned *timer_ctl = (unsigned *) (timer_base_addr + 0x08 + 0x20 * timer);
	volatile unsigned *timer_load = (unsigned *) (timer_base_addr + 0x20 * timer);

	timer_ctl_bkp[timer] = *timer_ctl;
	timer_load_bkp[timer] = *timer_load;

	*timer_ctl &= ~(1 << 7);
	*timer_ctl = 0b01100011;
	*timer_ctl |= (1 << 7);
}

void timer_restore(unsigned timer)
{
	volatile unsigned *timer_ctl = (unsigned *) (timer_base_addr + 0x08 + 0x20 * timer);
	volatile unsigned *timer_load = (unsigned *) (timer_base_addr + 0x20 * timer);

	*timer_ctl &= ~(1 << 7);
	*timer_ctl = timer_ctl_bkp[timer] & ~(1 << 7);
	*timer_load = timer_load_bkp[timer];
	*timer_ctl = timer_ctl_bkp[timer];
}

void timer_load(unsigned timer, unsigned value)
{
	volatile unsigned *timer_load = (unsigned *) (timer_base_addr + 0x20 * timer);
	*timer_load = value;
}

unsigned timer_read(unsigned timer)
{
	volatile unsigned *timer_value = (unsigned *) (timer_base_addr + 0x04 + 0x20 * timer);
	return *timer_value;
}
