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
	static constexpr int dim = 16;				 // side length
	static constexpr int size = dim * dim * dim; // volume

private:
	// Basic chunk attributes.
	// pos refers to the xyz coordinates of the block at
	// the chunk's -X -Y -Z corner.
	const VECTOR3 pos;
	std::array<Block, size> blocks;

	// Rendering implementation details
	static const std::array<VECTOR3, 8> corners;
	static const std::array<VECTOR3, 6> face_toplefts;
	static const std::array<VECTOR3, 6> face_u_orthos;
	static const std::array<VECTOR3, 6> face_v_orthos;

	// [[deprecated]] std::array<std::array<bool, 6>, size> occlusion_mask;
	// [[deprecated]] std::vector<VERTEX> vertices;
	// [[deprecated]] VECTOR3 prev_camera_pos;

	std::array<VECTOR3, (dim + 1)* (dim + 1)* (dim + 1)> projection_array;

	std::array<std::array<int, size>, 6> textures_by_dir;
	std::array<std::vector<IndexedVertex>, 6> iverts_by_dir;

	std::vector<IndexedVertex> indices;
	std::vector<VECTOR3> positions;
	std::vector<ProcessedPosition> processed;

	// Helper functions
	static VECTOR3 coords_of_idx(int idx);
	static int coords_to_idx(VECTOR3 coords);

	Block* block_at(int x, int y, int z);
	const Block* block_at(int x, int y, int z) const;

	bool block_is_visible_from_side(int idx, int face);

	std::array<IndexedVertex, 4> get_ivert_quad(
		VECTOR3 coords,
		blocktype_t btype, int face,
		int u, int v);

	void update_textures_by_dir();
	void update_iverts_by_dir();

	// The limit of block sizes that we render with greedy meshes.
	// For example, with greed_limit 2, we will combine 2x2 faces
	// into a single quad.
	//
	// NOTE: greed_limit > 4 results in out-of-bounds memory access, and
	//		so we don't allow it.
	// NOTE: Never change this directly! Use set_greed_limit() instead
	int greed_limit = 1;

	bool using_textures = true;

	// [[deprecated]] void update_occlusion_mask();
	// [[deprecated]] void update_vertices(VECTOR3 camera_pos);
	// [[deprecated]] int _render_old(VECTOR3 camera_pos);

public:
	CubicChunk(VECTOR3 pos);

	void set_block(int x, int y, int z, blocktype_t block_id);

	int render(VECTOR3 camera_pos, std::stringstream& ss, Stopwatch& stopwatch);

	void set_greed_limit(int limit);
	int get_greed_limit() { return greed_limit; }

	void enable_textures();
	void disable_textures();

	GLFix taxidist_to(VECTOR3 point);

	// This is still public because Block uses it (deprecated code)
	static constexpr unsigned int xyz_to_vert_idx(int x, int y, int z)
	{
		return x + y * (dim + 1) + z * (dim + 1) * (dim + 1);
	}
};

/*

	To render our chunk, we need to know what vertices to render.
	Since many of our vertices have the same positions, we can avoid a lot of
		extra math by calculating the processed positions beforehand and then
		using the nglDrawArray() function instead.

	For performance reasons, we should only render vertices that:
		- Are not completely occluded (updated when block data changes)
		- Are visible from the camera (updated when camera pos changes)

	Old pipeline:
		- Update 'occlusion_mask' when block data changes with
			update_occlusion_mask()
		- Update 'indices' when camera moves with
			update_vertices()
		- render_new() every frame

	New pipeline:
		TODO (it's done, i just need to update the comments)

*/
