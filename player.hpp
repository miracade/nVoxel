// player.hpp

#pragma once

#include "nGL/gl.h"

#include "touchpad.hpp"
#include "block.hpp"

class Player
{
public:
	VECTOR3 pos = {0, 0, 0};
	VECTOR3 angle = {0, 0, 0};
	static constexpr GLFix move_speed = Block::block_size * 8;

	void update(double dt_ms, const Touchpad& touchpad);
};