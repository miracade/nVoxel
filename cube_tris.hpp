// cube.hpp

#pragma once

#include <vector>

#include "nGL/gl.h"

template <int S, int T>
std::vector<VERTEX> cube_tris = 
{
	{0, 0, 0, 0, 0, 0xFFFF},
	{S, 0, 0, T, 0, 0xFFFF},
	{S, S, 0, T, T, 0xFFFF},
	{0, 0, 0, 0, 0, 0xFFFF},
	{S, S, 0, T, T, 0xFFFF},
	{0, S, 0, 0, T, 0xFFFF},

	{0, 0, 0, 0, 0, 0xFFFF},
	{S, 0, 0, T, 0, 0xFFFF},
	{S, 0, S, T, T, 0xFFFF},
	{0, 0, 0, 0, 0, 0xFFFF},
	{S, 0, S, T, T, 0xFFFF},
	{0, 0, S, 0, T, 0xFFFF},

	{0, 0, 0, 0, 0, 0xFFFF},
	{0, S, 0, T, 0, 0xFFFF},
	{0, S, S, T, T, 0xFFFF},
	{0, 0, 0, 0, 0, 0xFFFF},
	{0, S, S, T, T, 0xFFFF},
	{0, 0, S, 0, T, 0xFFFF},

	{0, 0, S, 0, 0, 0xFFFF},
	{S, 0, S, T, 0, 0xFFFF},
	{S, S, S, T, T, 0xFFFF},
	{0, 0, S, 0, 0, 0xFFFF},
	{S, S, S, T, T, 0xFFFF},
	{0, S, S, 0, T, 0xFFFF},

	{0, S, 0, 0, 0, 0xFFFF},
	{S, S, 0, T, 0, 0xFFFF},
	{S, S, S, T, T, 0xFFFF},
	{0, S, 0, 0, 0, 0xFFFF},
	{S, S, S, T, T, 0xFFFF},
	{0, S, S, 0, T, 0xFFFF},

	{S, 0, 0, 0, 0, 0xFFFF},
	{S, S, 0, T, 0, 0xFFFF},
	{S, S, S, T, T, 0xFFFF},
	{S, 0, 0, 0, 0, 0xFFFF},
	{S, S, S, T, T, 0xFFFF},
	{S, 0, S, 0, T, 0xFFFF},
};


