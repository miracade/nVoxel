
#pragma once

#include "libndls.h"

class Touchpad
{
private:
	touchpad_report_t report;

public:
	void update() { touchpad_scan(&report); }

	bool is_touched() const { return report.contact; }
	bool is_pressed() const { return report.pressed; }

	int16_t get_x() const { return report.x; }
	int16_t get_y() const { return report.y; }

	signed char get_x_vel() const { return report.x_velocity; }
	signed char get_y_vel() const { return report.y_velocity; }

	int16_t get_proximity() const { return report.proximity; }
	unsigned char get_arrow() const { return report.arrow; }

	const touchpad_report_t& get_report() const { return report; }
};

