// chunk.cpp

#include "chunk.hpp"

#include <functional>
#include <random>
#include <sstream>

#include "nGL/fastmath.h"

static std::mt19937 rng;

const std::array<VECTOR3, 8> CubicChunk::corners = {
	VECTOR3{  0,   0,   0}, VECTOR3{dim,   0,   0}, VECTOR3{  0, dim,   0}, VECTOR3{dim, dim,   0},
	VECTOR3{  0,   0, dim}, VECTOR3{dim,   0, dim}, VECTOR3{  0, dim, dim}, VECTOR3{dim, dim, dim},
};

CubicChunk::CubicChunk(VECTOR3 pos) : pos(pos)
{
	for (unsigned int i = 0; i < blocks.size(); ++i)
	{
		Block& block = blocks[i];
		// VECTOR3 block_coords = coords_of_idx(i);
		// GLFix x = pos.x + block_coords.x;
		// GLFix y = pos.y + block_coords.y;
		// GLFix z = pos.z + block_coords.z;
		// GLFix ground = fast_sin((x * 5 + z * 10).normaliseAngle()) * 2 + 4;
		// blocktype_t type = y > ground ? 0 : (y > ground - 1 ? 2 : 1);
		// blocktype_t type = rng() % 2 ? 2 : 0;
		blocktype_t type = 1;
		block.set_type(type);
	}
	update_visibility_mask();
}


VECTOR3 CubicChunk::coords_of_idx(int idx) const
{
	int x = idx % dim;
	int y = (idx / dim) % dim;
	int z = idx / (dim * dim);
	return VECTOR3{x, y, z};
}

int CubicChunk::coords_to_idx(VECTOR3 coords) const
{
	return coords.x + coords.y * dim + coords.z * dim * dim;
}


Block* CubicChunk::block_at(int x, int y, int z)
{
	if (x < 0 || x >= dim || y < 0 || y >= dim || z < 0 || z >= dim) return nullptr;
	return &blocks[coords_to_idx({x, y, z})];
}

const Block* CubicChunk::block_at(int x, int y, int z) const
{
	if (x < 0 || x >= dim || y < 0 || y >= dim || z < 0 || z >= dim) return nullptr;
	return &blocks[coords_to_idx({x, y, z})];
}

void CubicChunk::set_block(int x, int y, int z, blocktype_t block_id)
{
	Block* block = block_at(x, y, z);
	if (block == nullptr) return;
	block->set_type(block_id);
	update_visibility_mask();
}


void CubicChunk::update_visibility_mask()
{
	// This function should be run whenever the `block` array changes

	// This function updates the visibility mask for each block in the chunk.
	// Each element in the visibility_mask corresponds to a block in the chunk.
	// Each element in the visibility_mask is an array of 6 booleans, one for each face.
	// The faces are: -X, +X, -Y, +Y, -Z, +Z
	// The boolean value indicates whether the face is visible or not.

	// If the block's type is 0, it is invisible and all faces are hidden.
	// Otherwise:
	// 	Use the coords_of_idx function to get the XYZ coordinates of the block.
	//  If a face of a block is on the edge of the chunk, it is visible.
	//  Otherwise, check the block in the direction of the face.
	//  If the block in the direction of the face is invisible, the face is visible.
	//  Otherwise, the face is hidden.

	for (int i = 0; i < size; i++)
	{
		VECTOR3 coords = coords_of_idx(i);
		Block* block = block_at(coords.x, coords.y, coords.z);
		if (block->get_type() == 0)
		{
			visibility_mask[i] = {false, false, false, false, false, false};
			continue;
		}

		// -X
		if (coords.x == GLFix{0}) visibility_mask[i][0] = true;
		else visibility_mask[i][0] = block_at(coords.x - 1, coords.y, coords.z)->get_type() == 0;

		// +X
		if (coords.x == GLFix{dim - 1}) visibility_mask[i][1] = true;
		else visibility_mask[i][1] = block_at(coords.x + 1, coords.y, coords.z)->get_type() == 0;

		// -Y
		if (coords.y == GLFix{0}) visibility_mask[i][2] = true;
		else visibility_mask[i][2] = block_at(coords.x, coords.y - 1, coords.z)->get_type() == 0;

		// +Y
		if (coords.y == GLFix{dim - 1}) visibility_mask[i][3] = true;
		else visibility_mask[i][3] = block_at(coords.x, coords.y + 1, coords.z)->get_type() == 0;

		// -Z
		if (coords.z == GLFix{0}) visibility_mask[i][4] = true;
		else visibility_mask[i][4] = block_at(coords.x, coords.y, coords.z - 1)->get_type() == 0;

		// +Z
		if (coords.z == GLFix{dim - 1}) visibility_mask[i][5] = true;
		else visibility_mask[i][5] = block_at(coords.x, coords.y, coords.z + 1)->get_type() == 0;
	}
}


void CubicChunk::update_vertices(VECTOR3 camera_pos)
{
	vertices.clear();
	indices.clear();

	for (int i = 0; i < size; ++i)
	{
		VECTOR3 block_pos = coords_of_idx(i);
		block_pos.x += pos.x;
		block_pos.y += pos.y;
		block_pos.z += pos.z;
		block_pos.x *= Block::block_size;
		block_pos.y *= Block::block_size;
		block_pos.z *= Block::block_size;
		std::array<bool, 6> mask = {
			visibility_mask[i][0] && (block_pos.x > camera_pos.x),
			visibility_mask[i][1] && (block_pos.x < camera_pos.x + Block::block_size),
			visibility_mask[i][2] && (block_pos.y > camera_pos.y),
			visibility_mask[i][3] && (block_pos.y < camera_pos.y + Block::block_size),
			visibility_mask[i][4] && (block_pos.z > camera_pos.z),
			visibility_mask[i][5] && (block_pos.z < camera_pos.z + Block::block_size),
		};
		// blocks[i].write_vertices(vertices, block_pos, mask);
		blocks[i].write_ivertices(indices, coords_of_idx(i), mask);
	}
}

int CubicChunk::render(VECTOR3 camera_pos)
{
	if (camera_pos != prev_camera_pos)
	{
		update_vertices(camera_pos);
		prev_camera_pos = camera_pos;
	}

	indices.clear();
	positions.clear();

	// Iterate through vertices.
	// For each vertex:
	//	Its position is its x, y, and z members.
	//  If the vertex position is not in the positions vector, add it.
	//  Create an IndexedVertex with the index of the vertex's position in the positions vector, as well
	//	as the vertex's u, v, and c members.
	//	Add the IndexedVertex to the indices vector.

	for (VERTEX& vertex : vertices)
	{
		VECTOR3 pos = {vertex.x, vertex.y, vertex.z};
		auto it = std::find(positions.begin(), positions.end(), pos);
		unsigned int index;
		if (it == positions.end())
		{
			index = positions.size();
			positions.push_back(pos);
		}
		else
		{
			index = it - positions.begin();
		}
		IndexedVertex indexed_vertex = {index, vertex.u, vertex.v, vertex.c};
		indices.push_back(indexed_vertex);
	}

	processed.resize(positions.size());

	for (unsigned int i = 0; i < positions.size(); ++i)
	{
		processed[i].perspective_available = false;
		nglMultMatVectRes(transformation, &positions[i], &processed[i].transformed);
	}

	nglDrawArray(indices.data(), indices.size(), 
				 positions.data(), positions.size(),
				 processed.data(), 
				 GL_QUADS, false);

	// glBegin(GL_QUADS);
	// nglAddVertices(vertices.data(), vertices.size());
	// glEnd();

	return vertices.size();
}

int CubicChunk::render_new(VECTOR3 camera_pos, std::stringstream& ss, Stopwatch& stopwatch)
{
	// static std::map<VECTOR3, VECTOR3> projection_map;
	ss.str("");
	ss << stopwatch.get_ms() << "\n";
	ss << stopwatch.get_ms() << "\n";

	auto& vi = xyz_to_vert_idx;

	// Use matrix multiplication to transform the corners of the chunk into screen coordinates
	int out_of_bounds = 0;
	for (int i = 0; i < corners.size(); ++i)
	{
		VECTOR3 expanded_pos = (corners[i] + pos) * Block::block_size;

		// VECTOR3& processed_pos = projection_map[corners[i]];
		VECTOR3& processed_pos = projection_array[vi(corners[i].x, corners[i].y, corners[i].z)];
		nglMultMatVectRes(transformation, &expanded_pos, &processed_pos);
		if (processed_pos.z < GLFix{0} ||
			processed_pos.y / processed_pos.z > GLFix{1} || processed_pos.y / processed_pos.z < GLFix{-1} ||
			processed_pos.x / processed_pos.z > GLFix{1} || processed_pos.x / processed_pos.z < GLFix{-1})
		{
			++out_of_bounds;
		}
	}
	if (out_of_bounds == corners.size()) return 0;

	ss << stopwatch.get_ms() << "\n";


	for (int i = 0; i < corners.size(); i += 2)
	{
		// start and end will be the same except for the x-coordinate here.
		// v_start will be [0, y, z]
		// v_end will be [dim, y, z]
		const VECTOR3& v_start = corners[i];
		const VECTOR3& v_end = corners[i + 1];

		// const VECTOR3& p_start = projection_map[v_start];
		// const VECTOR3& p_end = projection_map[v_end];

		const VECTOR3& p_start = projection_array[vi(v_start.x, v_start.y, v_start.z)];
		const VECTOR3& p_end = projection_array[vi(v_end.x, v_end.y, v_end.z)];

		const VECTOR3 p_delta = (p_end - p_start) / dim;

		// We already have position values for [0, y, z] and [dim, y, z] so
		// now we linearly interpolate all the integral coordinates between
		// [1, y, z] to [dim - 1, y, z]
		for (int x = 1; x < dim; ++x)
		{
			VECTOR3 p = p_start + p_delta * x;
			VECTOR3 v = {x, v_start.y, v_start.z};
			// projection_map[v] = p;
			projection_array[vi(v.x, v.y, v.z)] = p;
		}
	}

	// Now projection_map contains all screen coordinates [x, y, z] where:
	// 		0 <= x <= dim; 		y == 0 || y == dim; 		z == 0 || z == dim;

	// Now we do the same thing for y.
	for (int i = 0; i < corners.size(); i += 4)
	{
		// start and end will be the same except for the y-coordinate here.
		// v_start will be [x, 0, z]
		// v_end will be [x, dim, z]
		const VECTOR3& v_start = corners[i];
		const VECTOR3& v_end = corners[i + 2];

		// Iterate through all x-values from 0 to dim (including dim!)
		for (int x = 0; x <= dim; ++x)
		{
			// const VECTOR3& p_start = projection_map[VECTOR3{x, v_start.y, v_start.z}];
			// const VECTOR3& p_end = projection_map[VECTOR3{x, v_end.y, v_end.z}];

			const VECTOR3& p_start = projection_array[vi(x, v_start.y, v_start.z)];
			const VECTOR3& p_end = projection_array[vi(x, v_end.y, v_end.z)];

			const VECTOR3 p_delta = (p_end - p_start) / dim;

			// We already have position values for [x, 0, z] and [x, dim, z] so
			// now we linearly interpolate all the integral coordinates between
			// [x, 1, z] to [x, dim - 1, z]
			for (int y = 1; y < dim; ++y)
			{
				VECTOR3 p = p_start + p_delta * y;
				VECTOR3 v = {x, y, v_start.z};
				// projection_map[v] = p;
				projection_array[vi(v.x, v.y, v.z)] = p;
			}
		}
	}

	// Now projection_map contains all screen coordinates [x, y, z] where:
	// 		0 <= x <= dim; 		0 <= y <= dim; 		z == 0 || z == dim;

	// Now all that's left is the z-values.
	{
		// start and end will be the same except for the z-coordinate here.
		// v_start will be [x, y, 0]
		// v_end will be [x, y, dim]
		const VECTOR3& v_start = corners[0];
		const VECTOR3& v_end = corners[4];

		// Iterate through all x-values from 0 to dim (including dim!)
		for (int x = 0; x <= dim; ++x)
		{
			// Iterate through all y-values from 0 to dim (including dim!)
			for (int y = 0; y <= dim; ++y)
			{
				// const VECTOR3& p_start = projection_map[VECTOR3{x, y, v_start.z}];
				// const VECTOR3& p_end = projection_map[VECTOR3{x, y, v_end.z}];
				const VECTOR3& p_start = projection_array[vi(x, y, v_start.z)];
				const VECTOR3& p_end = projection_array[vi(x, y, v_end.z)];

				const VECTOR3 p_delta = (p_end - p_start) / dim;

				// We already have position values for [x, y, 0] and [x, y, dim] so
				// now we linearly interpolate all the integral coordinates between
				// [x, y, 1] to [x, y, dim - 1]
				for (int z = 1; z < dim; ++z)
				{
					VECTOR3 p = p_start + p_delta * z;
					VECTOR3 v = {x, y, z};
					// projection_map[v] = p;
					projection_array[vi(v.x, v.y, v.z)] = p;
				}
			}
		}
	}

	// Now projection_map contains all screen coordinates [x, y, z] where:
	// 		0 <= x <= dim; 		0 <= y <= dim; 		0 <= z <= dim;
	// This should be (dim + 1)^3 vertices

	// debug << "Processed " << projection_map.size() << " vertices:\n";

	// // Print debug info
	// for (auto& [v, p] : projection_map)
	// {
	// 	debug << "[" << (int)v.x << " " << (int)v.y << " " << (int)v.z << "]->[" 
	// 	   << (int)p.x << " " << (int)p.y << " " << (int)p.z << "\n";
	// }

	// To use the nglDrawArray function, we need the following parameters:
	// 		indices: An array of IndexedVertex structs ( {index, u, v, c} )
	// 		positions: will be ignored since we've already processed the positions
	// 		processed: stores the processed positions. typically the nglDrawArray function
	// 			processes the positions for you, but since we've already done that, we
	//			just pass in an array of already-processed positions and then pass 
	//			false into the function's 'reset_processed' param

	ss << stopwatch.get_ms() << "\n";


	positions.clear();
	processed.clear();

	// for (auto& [c, p] : projection_map)
	// {
	// 	positions.push_back(c);
	// 	processed.push_back(ProcessedPosition{p, {0, 0, 0}, false});
	// }

	// TODO: optimize! this is no longer necessary now that i'm using an array instead of a map
	for (int z = 0; z < dim+1; ++z)
	{
		for (int y = 0; y < dim+1; ++y)
		{
			for (int x = 0; x < dim+1; ++x)
			{
				const VECTOR3 p = {x, y, z};
				positions.push_back(p);
				processed.push_back(ProcessedPosition{projection_array[vi(p.x, p.y, p.z)], {0, 0, 0}, false});
			}
		}
	}

	if (camera_pos != prev_camera_pos)
	{
		update_vertices(camera_pos);
		prev_camera_pos = camera_pos;
	}

	ss << stopwatch.get_ms() << "\n";


	// for (VERTEX vertex : vertices)
	// {
	// 	// debug << "Vertex: " << (int)vertex.x << " " << (int)vertex.y << " " << (int)vertex.z << "\n";
	// 	VECTOR3 vpos = VECTOR3{vertex.x, vertex.y, vertex.z} / Block::block_size - pos;
	// 	auto it = std::find(positions.begin(), positions.end(), vpos);
	// 	if (it == positions.end())
	// 	{
	// 		exit(EXIT_FAILURE);
	// 	}
	// 	unsigned int index = it - positions.begin();
	// 	indices.push_back(IndexedVertex{index, vertex.u, vertex.v, vertex.c});
	// }

	ss << stopwatch.get_ms() << "\n";


	// debug << "Drawing " << indices.size() << " indexed vertices\n";
	nglDrawArray(indices.data(), indices.size(),
				 positions.data(), positions.size(), 
				 processed.data(), GL_QUADS, 
				 false);	// false for 'clear_processed' param bc we've already processed the positions

	ss << stopwatch.get_ms() << "\n";


	return indices.size();
}