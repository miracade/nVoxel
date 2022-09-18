// block.cpp

#include "block.hpp"
#include "chunk.hpp"

constexpr COLOR color = 0xFFFF;
const std::array<VERTEX, 24> Block::origin_cube_vertices = {

	VERTEX(0         , 0         , block_size, 0       , 0       , color),
	VERTEX(0         , 0         , 0         , tex_size, 0       , color),
	VERTEX(0         , block_size, 0         , tex_size, tex_size, color),
	VERTEX(0         , block_size, block_size, 0       , tex_size, color),

	VERTEX(block_size, 0         , 0         , 0       , 0       , color),
	VERTEX(block_size, 0         , block_size, tex_size, 0       , color),
	VERTEX(block_size, block_size, block_size, tex_size, tex_size, color),
	VERTEX(block_size, block_size, 0         , 0       , tex_size, color),

	VERTEX(0         , 0         , 0         , 0       , 0       , color),
	VERTEX(0         , 0         , block_size, tex_size, 0       , color),
	VERTEX(block_size, 0         , block_size, tex_size, tex_size, color),
	VERTEX(block_size, 0         , 0         , 0       , tex_size, color),

	VERTEX(0         , block_size, 0         , 0       , 0       , color),
	VERTEX(block_size, block_size, 0         , tex_size, 0       , color),
	VERTEX(block_size, block_size, block_size, tex_size, tex_size, color),
	VERTEX(0         , block_size, block_size, 0       , tex_size, color),

	VERTEX(0         , 0         , 0         , 0       , 0       , color),
	VERTEX(block_size, 0         , 0         , tex_size, 0       , color),
	VERTEX(block_size, block_size, 0         , tex_size, tex_size, color),
	VERTEX(0         , block_size, 0         , 0       , tex_size, color),

	VERTEX(0         , 0         , block_size, 0       , 0       , color),
	VERTEX(block_size, 0         , block_size, tex_size, 0       , color),
	VERTEX(block_size, block_size, block_size, tex_size, tex_size, color),
	VERTEX(0         , block_size, block_size, 0       , tex_size, color),

};

auto& vi = CubicChunk::xyz_to_vert_idx;
const std::array<std::array<IndexedVertex, 4>, 6> Block::origin_cube_ivertices{{

	{
		IndexedVertex(vi(0, 0, 1), GLFix{0}, GLFix{0}, color),
		IndexedVertex(vi(0, 0, 0), tex_size, GLFix{0}, color),
		IndexedVertex(vi(0, 1, 0), tex_size, tex_size, color),
		IndexedVertex(vi(0, 1, 1), GLFix{0}, tex_size, color),
	}, {
		IndexedVertex(vi(1, 0, 0), GLFix{0}, GLFix{0}, color),
		IndexedVertex(vi(1, 0, 1), tex_size, GLFix{0}, color),
		IndexedVertex(vi(1, 1, 1), tex_size, tex_size, color),
		IndexedVertex(vi(1, 1, 0), GLFix{0}, tex_size, color),
	}, {
		IndexedVertex(vi(0, 0, 0), GLFix{0}, GLFix{0}, color),
		IndexedVertex(vi(0, 0, 1), tex_size, GLFix{0}, color),
		IndexedVertex(vi(1, 0, 1), tex_size, tex_size, color),
		IndexedVertex(vi(1, 0, 0), GLFix{0}, tex_size, color),
	}, {
		IndexedVertex(vi(0, 1, 0), GLFix{0}, GLFix{0}, color),
		IndexedVertex(vi(1, 1, 0), tex_size, GLFix{0}, color),
		IndexedVertex(vi(1, 1, 1), tex_size, tex_size, color),
		IndexedVertex(vi(0, 1, 1), GLFix{0}, tex_size, color),
	}, {
		IndexedVertex(vi(0, 0, 0), GLFix{0}, GLFix{0}, color),
		IndexedVertex(vi(1, 0, 0), tex_size, GLFix{0}, color),
		IndexedVertex(vi(1, 1, 0), tex_size, tex_size, color),
		IndexedVertex(vi(0, 1, 0), GLFix{0}, tex_size, color),
	}, {
		IndexedVertex(vi(0, 0, 1), GLFix{0}, GLFix{0}, color),
		IndexedVertex(vi(1, 0, 1), tex_size, GLFix{0}, color),
		IndexedVertex(vi(1, 1, 1), tex_size, tex_size, color),
		IndexedVertex(vi(0, 1, 1), GLFix{0}, tex_size, color),
	}

}};


void Block::write_vertices(std::vector<VERTEX>& vertices, 
						   VECTOR3 block_pos, std::array<bool, 6> faces) const
{
	for (unsigned int idx = 0; idx < origin_cube_vertices.size(); idx++)
	{
		if (faces[idx / 4] == false) continue; 
		VERTEX vertex = origin_cube_vertices[idx];
		vertices.emplace_back(
			vertex.x + block_pos.x, 
			vertex.y + block_pos.y, 
			vertex.z + block_pos.z, 
			vertex.u + type * static_cast<int>(tex_size), 
			vertex.v,
			vertex.c
		);
	}
}

void Block::write_ivertices(std::vector<IndexedVertex>& iverts, 
								   VECTOR3 subchunk_pos, std::array<bool, 6> faces) const
{
	// The indices for vertex positions are in a 3D array of size
	// 	(dim+1, dim+1, dim+1) where dim is the dimension of the chunk (CubicChunk::dim).

	for (int i = 0; i < 6; ++i)
	{
		if (faces[i] == false) continue;
		for (IndexedVertex vert : origin_cube_ivertices[i])
		{
			iverts.emplace_back(
				vert.index + vi(subchunk_pos.x, subchunk_pos.y, subchunk_pos.z),
				vert.u + type * static_cast<int>(tex_size),
				vert.v,
				vert.c
			);
		}
	}
}