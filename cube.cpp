// cube.cpp

#include "cube.hpp"

int Cube::load_vertices(const VECTOR3 camera_pos, VERTEX* vertex_ptr) const
{
	VERTEX* v = vertex_ptr;
	if (camera_pos.x < pos.x)
	{
		/*
			+Z  +
			+ZY +Y
		*/
		*v++ = VERTEX(pos.x, pos.y         , pos.z + size.z, u1, v1, 0xFFFF);
		*v++ = VERTEX(pos.x, pos.y         , pos.z         , u2, v1, 0xFFFF);
		*v++ = VERTEX(pos.x, pos.y + size.y, pos.z         , u2, v2, 0xFFFF);
		*v++ = VERTEX(pos.x, pos.y + size.y, pos.z + size.z, u1, v2, 0xFFFF);
	}
	else if (camera_pos.x > pos.x + size.x)
	{
		*v++ = VERTEX(pos.x + size.x, pos.y         , pos.z         , u1, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y         , pos.z + size.z, u2, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y + size.y, pos.z + size.z, u2, v2, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y + size.y, pos.z         , u1, v2, 0xFFFF);
	}

	if (camera_pos.y < pos.y)
	{
		*v++ = VERTEX(pos.x         , pos.y         , pos.z         , u1, v1, 0xFFFF);
		*v++ = VERTEX(pos.x         , pos.y         , pos.z + size.z, u2, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y         , pos.z + size.z, u2, v2, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y         , pos.z         , u1, v2, 0xFFFF);
	}
	else if (camera_pos.y > pos.y + size.y)
	{
		*v++ = VERTEX(pos.x         , pos.y + size.y, pos.z         , u1, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y + size.y, pos.z         , u2, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y + size.y, pos.z + size.z, u2, v2, 0xFFFF);
		*v++ = VERTEX(pos.x         , pos.y + size.y, pos.z + size.z, u1, v2, 0xFFFF);
	}

	if (camera_pos.z < pos.z)
	{
		*v++ = VERTEX(pos.x         , pos.y         , pos.z         , u1, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y         , pos.z         , u2, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y + size.y, pos.z         , u2, v2, 0xFFFF);
		*v++ = VERTEX(pos.x         , pos.y + size.y, pos.z         , u1, v2, 0xFFFF);
	}
	else if (camera_pos.z > pos.z + size.z)
	{
		*v++ = VERTEX(pos.x         , pos.y         , pos.z + size.z, u1, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y         , pos.z + size.z, u2, v1, 0xFFFF);
		*v++ = VERTEX(pos.x + size.x, pos.y + size.y, pos.z + size.z, u2, v2, 0xFFFF);
		*v++ = VERTEX(pos.x         , pos.y + size.y, pos.z + size.z, u1, v2, 0xFFFF);
	}

	return v - vertex_ptr;
}