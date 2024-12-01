#pragma once

#include <iostream>
#include "video_manager.h"
#include "glm/vec2.hpp"
#include "utilities/input_manager.h"
#include <glad/gl.h>

#include "miniaudio/miniaudio.h"
#include "project_settings.h"

class window {

public:
	window();

	~window();

	void window_loop();

	void close_window();

	video_manager manager{};

	project_settings settings{};

	std::unique_ptr<input::input_manager> input_man{nullptr};

	void reload_button_textures();

private:

	struct GLFWwindow* glfw_window{nullptr};

	uint16_t frame_width { 0 }; // vid_reader.height
	uint16_t frame_height { 0 }; // vid_reader.height

	GLuint video_texture;

	glm::vec2 vertices[4] = { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} };

	void render_window_bar();

	ma_result result;
	ma_engine engine;

	// This is a temp solution to forcing a video to play next frame.
	std::string force_video_to_play{};

	int button_texture_width { 0 };
	int button_texture_height { 0 };
	GLuint button_texture{};
	GLuint hovered_button_texture{};
	GLuint pressed_button_texture{};

};
