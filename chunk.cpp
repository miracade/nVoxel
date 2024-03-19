// chunk.cpp

#include "chunk.hpp"
#include "assets/colorsheet.hpp"
#include "nGL/fastmath.h"

#include <random>
#include <sstream>

static std::mt19937 rng;

CubicChunk::CubicChunk(VECTOR3 pos) : pos(pos)
{
	for (unsigned int i = 0; i < blocks.size(); ++i)
	{
		Block& block = blocks[i];
		VECTOR3 block_coords = coords_of_idx(i);
		int x = pos.x + block_coords.x;
		int y = pos.y + block_coords.y;
		int z = pos.z + block_coords.z;
		blocktype_t type = 3; //(x + y + z) / 16 % 2 + 1;
		bool exists = (fast_sin(GLFix(x * 14)) + fast_sin(GLFix(z * 19)) * 2) + 4 >= GLFix(y);
		block.set_type(type * exists);
	}
	// blocks[coords_to_idx({0, 0, 0})].set_type(1);
	// blocks[coords_to_idx({0, 0, 4})].set_type(2);
	// blocks[coords_to_idx({4, 0, 0})].set_type(3);
	// blocks[coords_to_idx({4, 0, 4})].set_type(4);
	update_textures_by_dir();
	update_iverts_by_dir();
}

const std::array<VECTOR3, 8> CubicChunk::corners = {
	VECTOR3{0, 0, 0},
	VECTOR3{dim, 0, 0},
	VECTOR3{0, dim, 0},
	VECTOR3{dim, dim, 0},
	VECTOR3{0, 0, dim},
	VECTOR3{dim, 0, dim},
	VECTOR3{0, dim, dim},
	VECTOR3{dim, dim, dim},
};

const std::array<VECTOR3, 6> CubicChunk::face_toplefts = {
	VECTOR3{0, 1, 1}, VECTOR3{1, 1, 0},
	VECTOR3{1, 0, 1}, VECTOR3{0, 1, 1},
	VECTOR3{0, 1, 0}, VECTOR3{1, 1, 1} };

const std::array<VECTOR3, 6> CubicChunk::face_u_orthos = {
	VECTOR3{0, 0, -1}, VECTOR3{0, 0, 1},
	VECTOR3{-1, 0, 0}, VECTOR3{1, 0, 0},
	VECTOR3{1, 0, 0}, VECTOR3{-1, 0, 0} };

const std::array<VECTOR3, 6> CubicChunk::face_v_orthos = {
	VECTOR3{0, -1, 0}, VECTOR3{0, -1, 0},
	VECTOR3{0, 0, -1}, VECTOR3{0, 0, -1},
	VECTOR3{0, -1, 0}, VECTOR3{0, -1, 0} };

VECTOR3 CubicChunk::coords_of_idx(int idx)
{
	int x = idx % dim;
	int y = (idx / dim) % dim;
	int z = idx / (dim * dim);
	return VECTOR3{ x, y, z };
}

int CubicChunk::coords_to_idx(VECTOR3 coords)
{
	return coords.x + coords.y * dim + coords.z * dim * dim;
}

Block* CubicChunk::block_at(int x, int y, int z)
{
	if (x < 0 || x >= dim ||
		y < 0 || y >= dim ||
		z < 0 || z >= dim)
		return nullptr;
	return &blocks[coords_to_idx({ x, y, z })];
}

const Block* CubicChunk::block_at(int x, int y, int z) const
{
	if (x < 0 || x >= dim ||
		y < 0 || y >= dim ||
		z < 0 || z >= dim)
		return nullptr;
	return &blocks[coords_to_idx({ x, y, z })];
}

bool CubicChunk::block_is_visible_from_side(int idx, int side)
{
	VECTOR3 coords = coords_of_idx(idx);
	switch (side)
	{
	case 0:
		--coords.x;
		break;
	case 1:
		++coords.x;
		break;
	case 2:
		--coords.y;
		break;
	case 3:
		++coords.y;
		break;
	case 4:
		--coords.z;
		break;
	case 5:
		++coords.z;
		break;
	}
	const Block* block = block_at(coords.x, coords.y, coords.z);
	return (block == nullptr || block->get_type() == 0);
}

std::array<IndexedVertex, 4> CubicChunk::get_ivert_quad(
	VECTOR3 coords,
	int tex, int face,
	int u, int v)
{
	VECTOR3 tl = face_toplefts[face] + coords;
	VECTOR3 tr = tl + face_u_orthos[face] * u;
	VECTOR3 br = tr + face_v_orthos[face] * v;
	VECTOR3 bl = tl + face_v_orthos[face] * v;

	int axis = face / 2;
	GLFix tex_u1 = Block::tex_size * axis * 4;
	GLFix tex_v1 = Block::tex_size * tex * 4;
	GLFix tex_u2 = tex_u1 + Block::tex_size * u;
	GLFix tex_v2 = tex_v1 + Block::tex_size * v;

	COLOR solid_color = using_textures ? 0 : texdata_colorsheet[tex * 3 + axis];

	return std::array<IndexedVertex, 4>{
		IndexedVertex{ xyz_to_vert_idx(tl.x, tl.y, tl.z), tex_u1, tex_v1, solid_color },
			IndexedVertex{ xyz_to_vert_idx(tr.x, tr.y, tr.z), tex_u2, tex_v1, solid_color },
			IndexedVertex{ xyz_to_vert_idx(br.x, br.y, br.z), tex_u2, tex_v2, solid_color },
			IndexedVertex{ xyz_to_vert_idx(bl.x, bl.y, bl.z), tex_u1, tex_v2, solid_color }};
}

void CubicChunk::update_textures_by_dir()
{
	// This function updates the textures_by_dir array and should
	// 	be called whenever the chunk's block data changes.

	// Loop through all blocks in the chunk
	for (int i = 0; i < size; ++i)
	{
		blocktype_t btype = blocks[i].get_type();

		if (btype == 0)
		{
			// If the block is air, don't render any of its faces
			for (std::array<int, size>& arr : textures_by_dir)
				arr[i] = 0;
		}
		else
		{
			// Otherwise, only render the faces that aren't occluded
			for (int face = 0; face < 6; ++face)
			{
				textures_by_dir[face][i] = block_is_visible_from_side(i, face) ? btype : 0;
			}
		}
	}
}

void CubicChunk::set_greed_limit(int limit)
{
	// if (limit > 4) limit = 4;
	if (limit < 1)
		limit = 1;
	if (greed_limit == limit)
		return;
	greed_limit = limit;
	update_iverts_by_dir();
}

void CubicChunk::enable_textures()
{
	if (using_textures)
		return;
	using_textures = true;
	update_iverts_by_dir();
}

void CubicChunk::disable_textures()
{
	if (!using_textures)
		return;
	using_textures = false;
	update_iverts_by_dir();
}

GLFix CubicChunk::taxidist_to(VECTOR3 point)
{
	VECTOR3 center = pos + VECTOR3{ dim / 2, dim / 2, dim / 2 };
	return (center.x - point.x).abs() + (center.y - point.y).abs() + (center.z - point.z).abs();
}

void CubicChunk::update_iverts_by_dir()
{
	// This function uses the textures_by_dir arrays to update the iverts_by_dir vectors.
	// 	Each element in the iverts_by_dir array is a vector of IndexedVertex structs.

	for (int face = 0; face < 6; ++face)
	{
		const auto& textures = textures_by_dir[face];
		auto& iverts = iverts_by_dir[face];
		iverts.clear();

		// face is a number from 0 to 5, representing which face of the block
		// 	we're working with. To combine textures and reduce the vertex count,
		//  we're trying to find adjacent faces with the same texture. However, since
		// 	different faces face different directions, we need to take this into account.

		// Given any coordinate `c` and its face:
		//	the block `c + w_dir`'s face will be to its right
		//	the block `c + h_dir`'s face will be to its bottom (since top-left is (0, 0))
		VECTOR3 w_dir = face_u_orthos[face];
		VECTOR3 h_dir = face_v_orthos[face];

		std::array<bool, size> ignore_mask;
		ignore_mask.fill(false);

		// Iterate through all blocks in the chunk
		for (int idx = 0; idx < size; ++idx)
		{
			if (ignore_mask[idx])
				continue;

			VECTOR3 coords = coords_of_idx(idx);

			int tex = textures[idx];
			if (tex == 0)
				continue;

			// Currently our texture is only a 1x1 block. Let's see if we can
			// combine it with any adjacent blocks to make a larger texture while
			// reducing the vertex count
			int ivert_w = 1;
			int ivert_h = 1;

			// Keep looking to the right of the current block.
			// If we find a block with the same texture, we can combine that one
			// 	to our current texture. (We then set the texture of that block to 0
			// 	so we don't render it multiple times.)
			// If the block doesn't exist, or if it has a different texture, we stop.

			VECTOR3 adj_coords = coords + w_dir;
			while (ivert_w < greed_limit)
			{
				if (adj_coords.x < GLFix{ 0 } || adj_coords.x >= dim ||
					adj_coords.y < GLFix{ 0 } || adj_coords.y >= dim ||
					adj_coords.z < GLFix{ 0 } || adj_coords.z >= dim)
					break;

				int next_idx = coords_to_idx({ adj_coords.x, adj_coords.y, adj_coords.z });
				if (next_idx >= size)
					break;

				int next_tex = textures[next_idx];
				if (next_tex != tex)
					break;

				ignore_mask[next_idx] = true;
				++ivert_w;
				adj_coords = adj_coords + w_dir;
			}

			// Begin by assuming that our ivert can have a height of greed_limit
			//	(this is optimal)
			ivert_h = greed_limit;

			// Iterate through columns of our current ivert
			for (int u = 0; u < ivert_w; ++u)
			{
				for (int v = 1; v < ivert_h; ++v)
				{
					// If we find in any column that ivert can't have our assumed
					// 	height, update ivert_h accordingly

					adj_coords = coords + (w_dir * u) + (h_dir * v);
					if (adj_coords.x < GLFix{ 0 } || adj_coords.x >= dim ||
						adj_coords.y < GLFix{ 0 } || adj_coords.y >= dim ||
						adj_coords.z < GLFix{ 0 } || adj_coords.z >= dim)
					{
						ivert_h = v;
						break;
					}

					int next_idx = coords_to_idx({ adj_coords.x, adj_coords.y, adj_coords.z });
					if (next_idx >= size)
					{
						ivert_h = v;
						break;
					}

					int next_tex = textures[next_idx];
					if (next_tex != tex)
					{
						ivert_h = v;
						break;
					}
				}
			}

			// Update the ignore_mask array accordingly so we don't render the same
			// 	face multiple times. We've already done this for the top row so
			// 	we're just doing it for the remaining ones
			for (int u = 0; u < ivert_w; ++u)
			{
				for (int v = 1; v < ivert_h; ++v)
				{
					adj_coords = coords + (w_dir * u) + (h_dir * v);
					int next_idx = coords_to_idx({ adj_coords.x, adj_coords.y, adj_coords.z });
					ignore_mask[next_idx] = true;
				}
			}

			// Now that we know how big our texture is, we can add the indexed vertices
			// 	to our iverts vector :)
			//  (the smiley face gets rid of all the bugs, trust me)
			auto ivert_quad = get_ivert_quad(coords, tex, face, ivert_w, ivert_h);
			for (const IndexedVertex& ivert : ivert_quad)
			{
				iverts.push_back(ivert);
			}
		}
	}
}

void CubicChunk::set_block(int x, int y, int z, blocktype_t block_id)
{
	Block* block = block_at(x, y, z);
	if (block == nullptr)
		return;
	block->set_type(block_id);
	// update_occlusion_mask();
}

int CubicChunk::render(VECTOR3 camera_pos, std::stringstream& ss, Stopwatch& stopwatch)
{
	// static std::map<VECTOR3, VECTOR3> projection_map;
	ss.str("");
	// ss << stopwatch.get_ms() << "\n";
	// ss << stopwatch.get_ms() << "\n";

	/// PART 0: Easy Optimization
	/// Use matrix multiplication to transform the corners of the chunk into screen coordinates.
	/// If ALL of the corners are out of bounds, we don't need to render the chunk.

	auto& vi = xyz_to_vert_idx;

	int out_of_bounds = 0;
	for (int i = 0; i < corners.size(); ++i)
	{
		VECTOR3 expanded_pos = (corners[i] + pos) * Block::block_size;

		// VECTOR3& processed_pos = projection_map[corners[i]];
		VECTOR3& processed_pos = projection_array[vi(corners[i].x, corners[i].y, corners[i].z)];
		nglMultMatVectRes(transformation, &expanded_pos, &processed_pos);
		if (processed_pos.z < GLFix{ 0 } ||
			processed_pos.y / processed_pos.z > GLFix{ 1 } || processed_pos.y / processed_pos.z < GLFix{ -1 } ||
			processed_pos.x / processed_pos.z > GLFix{ 1 } || processed_pos.x / processed_pos.z < GLFix{ -1 })
		{
			++out_of_bounds;
		}
	}
	if (out_of_bounds == corners.size())
		return 0;

	// ss << stopwatch.get_ms() << "\n";

	/// PART 1: Transforming Position Vectors (v_*) into Projection Vectors (p_*)
	///		AKA getting screen coordinates of vectors.
	/// 	We do lots of linear interpolation here so not everything is 100%
	///		accurate, but this seems to give us a >100% speedup, so we'll take it

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
			VECTOR3 v = { x, v_start.y, v_start.z };
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
				VECTOR3 v = { x, y, v_start.z };
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
					VECTOR3 v = { x, y, z };
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

	// ss << stopwatch.get_ms() << "\n";

	// for (auto& [c, p] : projection_map)
	// {
	// 	positions.push_back(c);
	// 	processed.push_back(ProcessedPosition{p, {0, 0, 0}, false});
	// }

	/// PART 2: Format our positions and processed positions for the nglDrawArray function call
	///
	/// To use the nglDrawArray function, we need the following parameters:
	/// 	indices: An array of IndexedVertex structs ( {index, u, v, c} )
	/// 	positions: will be ignored since we've already processed the positions
	/// 	processed: stores the processed positions. typically the nglDrawArray function
	/// 		processes the positions for you, but since we've already done that, we
	///		just pass in an array of already-processed positions and then pass
	///		false into the function's 'reset_processed' param

	// todo: optimize! this is no longer necessary now that i'm using an array instead of a map
	// edit: somehow this is actually FASTER than just iterating through the projection array
	// 	(i have no idea why, but we're keeping this i guess)
	positions.clear();
	processed.clear();
	for (int z = 0; z < dim + 1; ++z)
	{
		for (int y = 0; y < dim + 1; ++y)
		{
			for (int x = 0; x < dim + 1; ++x)
			{
				const VECTOR3 p = { x, y, z };
				positions.push_back(p);
				processed.push_back(ProcessedPosition{ projection_array[vi(p.x, p.y, p.z)], {0, 0, 0}, false });
			}
		}
	}

	// if (camera_pos != prev_camera_pos)
	// {
	// 	// update_vertices(camera_pos);
	// 	// update_occlusion_mask();
	// 	// update_textures_by_dir();
	// 	// update_iverts_by_dir();

	// 	indices.clear();
	// 	for (const std::vector<IndexedVertex>& iverts : iverts_by_dir)
	// 	{
	// 		int i = 0;
	// 		for (const IndexedVertex& ivert : iverts)
	// 		{
	// 			// ss << ivert.index << " ";
	// 			// if (++i % 4 == 0) ss << "\n";
	// 			indices.push_back(ivert);
	// 		}
	// 	}
	// 	prev_camera_pos = camera_pos;
	// }

	/// PART 3: Use all the data we have to make the `nglDrawArray` function call.
	///		We'll be drawing up to six faces of vertices, since the camera
	///			could be in the chunk we're drawing.
	///		The iverts are already generated from the `update_iverts_by_dir` call.
	///		When we're all done, we return the number of faces we drew

	std::array<bool, 6> drawn_faces = {
		(camera_pos.x / Block::block_size < pos.x + dim),
		(camera_pos.x / Block::block_size > pos.x),
		(camera_pos.y / Block::block_size < pos.y + dim),
		(camera_pos.y / Block::block_size > pos.y),
		(camera_pos.z / Block::block_size < pos.z + dim),
		(camera_pos.z / Block::block_size > pos.z) };

	const TEXTURE* texture = nglGetTexture();
	if (!using_textures)
		glBindTexture(nullptr);

	int draw_count = 0;
	for (int dir = 0; dir < 6; ++dir)
	{
		if (!drawn_faces[dir])
			continue;
		const std::vector<IndexedVertex>& iverts = iverts_by_dir[dir];
		nglDrawArray(iverts.data(), iverts.size(),
			positions.data(), positions.size(),
			processed.data(), GL_QUADS,
			false); // false for 'clear_processed' param bc we've already processed the positions
		draw_count += iverts.size();
	}

	// ss << stopwatch.get_ms() << "\n";

	// nglDrawArray(indices.data(), indices.size(),
	// 			 positions.data(), positions.size(),
	// 			 processed.data(), GL_QUADS,
	// 			 false);	// false for 'clear_processed' param bc we've already processed the positions

	// ss << stopwatch.get_ms() << "\n";
	glBindTexture(texture);
	return draw_count;
}

// LEGACY CODE FROM HERE ON OUT.
// This code is marked as [[deprecated]] in the header file for the
//	compiler.

/*

[[deprecated]]
void CubicChunk::update_occlusion_mask()
{
	// This function should be run whenever the `block` array changes

	// This function updates the occlusion mask for each block in the chunk.
	// Each element in the occlusion_mask corresponds to a block in the chunk.
	// Each element in the occlusion_mask is an array of 6 booleans, one for each face.
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
			occlusion_mask[i] = {false, false, false, false, false, false};
			continue;
		}

		// -X
		if (coords.x == GLFix{0}) occlusion_mask[i][0] = true;
		else occlusion_mask[i][0] = block_at(coords.x - 1, coords.y, coords.z)->get_type() == 0;

		// +X
		if (coords.x == GLFix{dim - 1}) occlusion_mask[i][1] = true;
		else occlusion_mask[i][1] = block_at(coords.x + 1, coords.y, coords.z)->get_type() == 0;

		// -Y
		if (coords.y == GLFix{0}) occlusion_mask[i][2] = true;
		else occlusion_mask[i][2] = block_at(coords.x, coords.y - 1, coords.z)->get_type() == 0;

		// +Y
		if (coords.y == GLFix{dim - 1}) occlusion_mask[i][3] = true;
		else occlusion_mask[i][3] = block_at(coords.x, coords.y + 1, coords.z)->get_type() == 0;

		// -Z
		if (coords.z == GLFix{0}) occlusion_mask[i][4] = true;
		else occlusion_mask[i][4] = block_at(coords.x, coords.y, coords.z - 1)->get_type() == 0;

		// +Z
		if (coords.z == GLFix{dim - 1}) occlusion_mask[i][5] = true;
		else occlusion_mask[i][5] = block_at(coords.x, coords.y, coords.z + 1)->get_type() == 0;
	}
}

[[deprecated]]
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
			occlusion_mask[i][0] && (block_pos.x > camera_pos.x),
			occlusion_mask[i][1] && (block_pos.x < camera_pos.x + Block::block_size),
			occlusion_mask[i][2] && (block_pos.y > camera_pos.y),
			occlusion_mask[i][3] && (block_pos.y < camera_pos.y + Block::block_size),
			occlusion_mask[i][4] && (block_pos.z > camera_pos.z),
			occlusion_mask[i][5] && (block_pos.z < camera_pos.z + Block::block_size),
		};
		// blocks[i].write_vertices(vertices, block_pos, mask);
		blocks[i].write_ivertices(indices, coords_of_idx(i), mask);
	}
}

[[deprecated]]
int CubicChunk::_render_old(VECTOR3 camera_pos)
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

*/
