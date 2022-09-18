
#include <string>
#include <sstream>

#include <os.h>
#include <libndls.h>

#include "nGL/gl.h"

#include "assets/ascii.hpp"
#include "assets/blocks.hpp"

#include "timer.hpp"
#include "running_average.hpp"
#include "touchpad.hpp"

#include "player.hpp"
#include "chunk.hpp"

void dither_z_buffer(const uint16_t max_dist)
{
    static constexpr uint16_t dither_tile[4][4] = {
        {0, 4, 2, 4},
        {7, 3, 6, 3},
        {2, 4, 1, 4},
        {6, 3, 5, 3},
    };

	uint16_t* z_buffer = glGetZBuffer();

	for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                z_buffer[y*SCREEN_WIDTH + x] = max_dist - (dither_tile[y%4][x%4] * 8);
            }
        }
}

int main()
{
	nglInit();
	glBindTexture(&tex_blocks);

	static COLOR frame_buffer[320 * 240];
	// static COLOR frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	nglSetBuffer(frame_buffer);

	std::remove_reference_t<decltype(*glGetZBuffer())> z_fog = Block::block_size * 32;

	int ms_since_last_input = 0;

	Touchpad touchpad;
	Player player;
	player.pos = {Block::block_size * CubicChunk::dim * 1, 0, Block::block_size * CubicChunk::dim * -2};

	std::vector<CubicChunk> chunks;
	CubicChunk chunk{VECTOR3{0, 0, 0}};
	chunks.push_back(chunk);
	// for (int z = 0; z < 6; z++)
	// {
	// 	for (int x = 0; x < 6; x++)
	// 	{
	// 		CubicChunk chunk{VECTOR3{x * CubicChunk::dim, 0, z * CubicChunk::dim}};
	// 		chunks.push_back(chunk);
	// 	}
	// }

	Stopwatch total_stopwatch;
	total_stopwatch.start();

	Stopwatch lap_stopwatch;
	RunningAverage<double, 8> frame_times{0};
	double dt_ms = 0;

	std::stringstream debug_info;

	unsigned int frame = 0;
	while (!isKeyPressed(KEY_NSPIRE_ESC))
	{
		frame++; 

		// Updating the touchpad *after* starting the stopwatch makes the touchpad input
		// stop working for some reason, so we have to do it before
		touchpad.update();
		lap_stopwatch.start();

		if (any_key_pressed() || touchpad.is_touched()) 
			ms_since_last_input = 0; 
		else 
			ms_since_last_input += dt_ms;

		glPushMatrix();

		glColor3f(0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// dither_z_buffer(z_fog);

		player.update(dt_ms, touchpad);

		nglRotateX(GLFix{360} - player.angle.x);	// Invert the angle, which is in [0-360)
		nglRotateY(GLFix{360} - player.angle.y);
		glTranslatef(-player.pos.x, -player.pos.y, -player.pos.z);

		int vertex_count = 0;
		for (CubicChunk& chunk : chunks)
		{
			// vertex_count += chunk.render(player.pos);
			vertex_count += chunk.render_new(player.pos, debug_info, lap_stopwatch);
			// if (lap_stopwatch.get_ms() > (1000 / 12)) break;
		}

		if (frame)
		{
			// debug_info.str("");
			debug_info << static_cast<int>(1000.0f / frame_times.get<double>()) << "FPS\n";
			debug_info << frame_times.get<int>() << " mspt\n";
			debug_info << vertex_count << " verts\n";
			
			// VECTOR3 c = {0, 0, 0};
			// VECTOR3 p;
			// nglMultMatVectRes(transformation, &c, &p);
			// debug_info << (int)p.x << " " << (int)p.y << " " << (int)p.z << "\n";
			// debug_info << "D=" << downsampling;
		}

		glPopMatrix();

		if constexpr (SCREEN_WIDTH == 160)
		{
			for (int y = 240-1; y >= 0; --y)
			{
				for (int x = 320-1; x >= 0; --x)
				{
					int src_y = y/2;
					int src_x = x/2;
					frame_buffer[y*320 + x] = frame_buffer[src_y*160 + src_x];
				}
			}
		}

		font_print(frame_buffer, debug_info.str(), 2, 2, 0x0);

		nglDisplay();

		dt_ms = lap_stopwatch.get_ms();
		frame_times.add(dt_ms);
	}
	
	nglUninit();
	// delete[] frame_buffer;

	return 0;
}