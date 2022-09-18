// block.hpp

#pragma once

#include <array>
#include <vector>

#include "nGL/gl.h"
#include "nGL/gldrawarray.h"	// IndexedVertex

using blocktype_t = int;

class Block
{
public:
	static constexpr int block_size = 32;
	static constexpr GLFix tex_size = 16;

private:
	static const std::array<VERTEX, 24> origin_cube_vertices;
	static const std::array<std::array<IndexedVertex, 4>, 6> origin_cube_ivertices;
	blocktype_t type;

public:
	Block() : type{0} {};
	Block(blocktype_t type) : type{type} {};

	blocktype_t get_type() const { return type; }
	void set_type(blocktype_t type) { this->type = type; }
	void write_vertices(std::vector<VERTEX>& vertices, VECTOR3 block_pos, std::array<bool, 6> faces) const;

	void write_ivertices(std::vector<IndexedVertex>& iverts, VECTOR3 subchunk_pos, std::array<bool, 6> faces) const;
};

