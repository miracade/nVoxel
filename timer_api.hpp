
#pragma once

void timer_init(unsigned timer);

void timer_restore(unsigned timer);

void timer_load(unsigned timer, unsigned value);

unsigned timer_read(unsigned timer);