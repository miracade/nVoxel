
#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <memory>

#include "nGL/gl.h"

GLFix midpoint(GLFix a, GLFix b)
{
	// Marginally slower than (a+b)/2, but doesn't overflow
	return a + ((b - a) / 2);
}

VERTEX midpoint(VERTEX a, VERTEX b)
{
	return {
		midpoint(a.x, b.x),
		midpoint(a.y, b.y),
		midpoint(a.z, b.z),
		midpoint(a.u, b.u),
		midpoint(a.v, b.v),
		a.c
	};
}

std::vector<VERTEX> split_tris(const std::vector<VERTEX>& vertices)
{
	if (vertices.size() % 3 != 0)
	{
		throw std::runtime_error("vertices.size() % 3 != 0");
	}

	std::vector<VERTEX> result(vertices.size() * 4);
	

	for (size_t i = 0; i < vertices.size(); i += 3)
	{
		const VERTEX& a = vertices[i];
		const VERTEX& b = vertices[i + 1];
		const VERTEX& c = vertices[i + 2];
		
		VERTEX ab = midpoint(a, b);
		VERTEX bc = midpoint(b, c);
		VERTEX ca = midpoint(c, a);

		result[i*4 + 0] = a; 	result[i*4 + 1] = ab; 	result[i*4 + 2] = ca;
		result[i*4 + 3] = ab;	result[i*4 + 4] = b ;	result[i*4 + 5] = ca;
		result[i*4 + 6] = ca;	result[i*4 + 7] = b ;	result[i*4 + 8] = bc;
		// result[i*4 + 3] = ab;	result[i*4 + 4] = b ;	result[i*4 + 5] = bc;
		// result[i*4 + 6] = ab;	result[i*4 + 7] = bc;	result[i*4 + 8] = ca;
		result[i*4 + 9] = ca; 	result[i*4 + 10] = bc;	result[i*4 + 11] = c;
	}

	return result;
}