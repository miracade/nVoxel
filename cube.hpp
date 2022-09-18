// cube.hpp

#pragma once

#include "nGL/gl.h"

// The Cube class represents an axis-aligned cube with a given pos and size.
class Cube
{
private:
	GLFix u1 = 0;
	GLFix u2 = 16;
	GLFix v1 = 0;
	GLFix v2 = 16;

public:
	VECTOR3 pos;
	VECTOR3 size;

	Cube(VECTOR3 pos, VECTOR3 size) : pos(pos), size(size) {};

	// Given a camera position and a pointer to an VERTEX[], populates the array with
	// culled vertices of the cube. Returns the number of vertices in the array.
	int load_vertices(const VECTOR3 camera_pos, VERTEX* vertex_ptr) const;
};