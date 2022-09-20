// chunk.hpp

#pragma once

#include <vector>

#include "nGL/gl.h"
#include "nGL/gldrawarray.h"

#include "block.hpp"
#include "timer.hpp"

class CubicChunk
{
public:
	static constexpr int dim = 8;	// side length
	static constexpr int size = dim * dim * dim;	// volume
	static constexpr unsigned int xyz_to_vert_idx(int x, int y, int z) {
		return x + y * (dim + 1) + z * (dim + 1) * (dim + 1);
	}
	
private:
	const VECTOR3 pos;
	std::array<Block, size> blocks;
	std::array<std::array<bool, 6>, size> visibility_mask;

	std::vector<VERTEX> vertices;
	VECTOR3 prev_camera_pos;

	static const std::array<VECTOR3, 8> corners;
	std::array<VECTOR3, (dim+1)*(dim+1)*(dim+1)> projection_array;
	
	std::vector<IndexedVertex> indices;
	std::vector<VECTOR3> positions;
	std::vector<ProcessedPosition> processed;

public:
	CubicChunk(VECTOR3 pos);

	VECTOR3 coords_of_idx(int idx) const;
	int coords_to_idx(VECTOR3 coords) const;

	Block* block_at(int x, int y, int z);
	const Block* block_at(int x, int y, int z) const;

	void set_block(int x, int y, int z, blocktype_t block_id);
	void update_visibility_mask();

	void update_vertices(VECTOR3 camera_pos);
	int render(VECTOR3 camera_pos);

	int render_new(VECTOR3 camera_pos, std::stringstream& ss, Stopwatch& stopwatch);

};
