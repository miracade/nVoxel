
#include <string>
#include <sstream>

#include <os.h>
#include <libndls.h>

#include "nGL/gl.h"

#include "assets/ascii.hpp"
#include "assets/spritesheet.hpp"

#include "timer.hpp"
#include "running_average.hpp"
#include "touchpad.hpp"

#include "player.hpp"
#include "chunk.hpp"

// NOTE: currently this function is horrendously slow, using around 5ms (really???)
// 		If we pre-generate the dithered z-buffer and just copy it over each frame, we could
// 		potentially save a lot of time. Otherwise, definitely not worth it
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
			z_buffer[y * SCREEN_WIDTH + x] = max_dist - (dither_tile[y % 4][x % 4] * 8);
		}
	}
}

int main()
{
	GLFix texture_render_dist = 16;
	nglInit();
	glBindTexture(&tex_spritesheet);

	// static COLOR frame_buffer[320 * 240];
	static COLOR frame_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
	nglSetBuffer(frame_buffer);

	// using z_buffer_t = std::remove_reference_t<decltype(*glGetZBuffer())>;
	// z_buffer_t z_fog = Block::block_size * 32;

	int ms_since_last_input = 0;

	Touchpad touchpad;
	Player player;
	player.pos = { Block::block_size * CubicChunk::dim * 1, 0, Block::block_size * CubicChunk::dim * -2 };

	std::vector<CubicChunk> chunks;
	// CubicChunk chunk{VECTOR3{0, 0, 0}};
	// chunks.push_back(chunk);
	for (int z = 0; z < 1; z++)
	{
		for (int y = 0; y < 1; y++)
		{
			for (int x = 0; x < 1; x++)
			{
				CubicChunk chunk{
					VECTOR3{x * CubicChunk::dim,
							y * CubicChunk::dim,
							z * CubicChunk::dim}
				};
				chunks.push_back(chunk);
			}
		}
	}

	Stopwatch total_stopwatch;
	total_stopwatch.start();

	Stopwatch lap_stopwatch;
	RunningAverage<double, 8> frame_times{ 0 };
	double dt_ms = 0;

	std::stringstream debug_info;

	int resolution_options[] = { 80, 160, 320 };
	int resolution_index = 2;

	unsigned int frame = 0;
	while (!isKeyPressed(KEY_NSPIRE_ESC))
	{
		frame++;

		debug_info.str("");

		// Updating the touchpad *after* starting the stopwatch makes the touchpad input
		// stop working for some reason, so we have to do it before
		touchpad.update();
		lap_stopwatch.start();

		debug_info << "start:" << lap_stopwatch.get_ms() << "\n";


		// Gather player inputs

		// if (isKeyPressed(KEY_NSPIRE_A))
		// 	for (CubicChunk& chunk : chunks)
		// 		chunk.set_greed_limit(chunk.get_greed_limit() - 1);
		// if (isKeyPressed(KEY_NSPIRE_S))
		// 	for (CubicChunk& chunk : chunks)
		// 		chunk.set_greed_limit(chunk.get_greed_limit() + 1);

		if (isKeyPressed(KEY_NSPIRE_D)) {
			texture_render_dist -= 1;
		}
		if (isKeyPressed(KEY_NSPIRE_F)) {
			texture_render_dist += 1;
		}
		if (isKeyPressed(KEY_NSPIRE_R))
			resolution_index = (resolution_index + 1) % 3;

		if (any_key_pressed() || touchpad.is_touched())
			ms_since_last_input = 0;
		else
			ms_since_last_input += dt_ms;

		debug_info << "input:" << lap_stopwatch.get_ms() << "\n";

		glSetDrawResolution(resolution_options[resolution_index]);

		glPushMatrix();

		glColor3f(0.4f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// dither_z_buffer(z_fog);

		player.update(dt_ms, touchpad);

		nglRotateX(GLFix{ 360 } - player.angle.x);	// Invert the angle, which is in [0-360)
		nglRotateY(GLFix{ 360 } - player.angle.y);
		glTranslatef(-player.pos.x, -player.pos.y, -player.pos.z);

		debug_info << "render setup:" << lap_stopwatch.get_ms() << "\n";


		int vertex_count = 0;
		for (CubicChunk& chunk : chunks)
		{
			if (chunk.taxidist_to(player.pos / Block::block_size) > texture_render_dist) {

				chunk.disable_textures();
				chunk.set_greed_limit(4);
			}
			else {
				chunk.enable_textures();
				chunk.set_greed_limit(1);
			}
			vertex_count += chunk.render(player.pos, debug_info, lap_stopwatch);
			// if (lap_stopwatch.get_ms() > (1000 / 12)) break;
		}

		if (frame)
		{
			debug_info << static_cast<int>(1000.0f / frame_times.get<double>()) << "FPS; ";
			debug_info << frame_times.get<int>() << " mspt\n";

			debug_info << vertex_count << " verts\n";

			debug_info << "greed=" << chunks[0].get_greed_limit() << "; ";
			debug_info << "res=" << resolution_options[resolution_index] << "\n";
		}

		glPopMatrix();

		glUpscaleFrameBuffer();
		font_print(frame_buffer, debug_info.str(), 2, 2, 0x0);

		nglDisplay();

		dt_ms = lap_stopwatch.get_ms();
		frame_times.add(dt_ms);
	}

	nglUninit();

	return 0;
}
