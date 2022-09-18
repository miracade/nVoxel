// player.cpp

#include "player.hpp"

#include "block.hpp" 		// for block_size
#include "nGL/fastmath.h"

void Player::update(double dt_ms, const Touchpad& touchpad)
{
	const GLFix move_amt = move_speed * dt_ms / 1000;

	const GLFix yaw = angle.y;
	const VECTOR3 forward = {fast_sin(yaw), 0, fast_cos(yaw)};
	const VECTOR3 right = {fast_sin(yaw + 90), 0, fast_cos((yaw + 90).normaliseAngle())};

	VECTOR3 dpos = {0, 0, 0};

	if (isKeyPressed(KEY_NSPIRE_8))
	{
		dpos.x += forward.x * move_amt;
		dpos.z += forward.z * move_amt;
	}
	if (isKeyPressed(KEY_NSPIRE_2))
	{
		dpos.x -= forward.x * move_amt;
		dpos.z -= forward.z * move_amt;
	}
	if (isKeyPressed(KEY_NSPIRE_6))
	{
		dpos.x += right.x * move_amt;
		dpos.z += right.z * move_amt;
	}
	if (isKeyPressed(KEY_NSPIRE_4))
	{
		dpos.x -= right.x * move_amt;
		dpos.z -= right.z * move_amt;
	}

	if (isKeyPressed(KEY_NSPIRE_7))
	{
		dpos.y += move_amt;
	}
	if (isKeyPressed(KEY_NSPIRE_9))
	{
		dpos.y -= move_amt;
	}

	pos.x += dpos.x;
	pos.y += dpos.y;
	pos.z += dpos.z;

	// Rotating along the x-axis (pitch)
	angle.x -= touchpad.get_y_vel();
	// Clamp pitch so that the player can't look "upside down"
	if (angle.x > GLFix{90} && angle.x < GLFix{180}) angle.x = GLFix{90};
	if (angle.x > GLFix{180} && angle.x < GLFix{270}) angle.x = GLFix{270};
	angle.x.normaliseAngle();

	// Rotating along the y-axis (yaw)
	angle.y += touchpad.get_x_vel();
	angle.y.normaliseAngle();

}
