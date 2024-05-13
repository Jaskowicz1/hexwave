#pragma once

#include <iostream>
#include "video_manager.h"
#include "glm/vec2.hpp"
#include "utilities/input_manager.h"
#include <glad/gl.h>

class window {

public:
	window();

	~window();

	void window_loop();

	void close_window();

	video_manager manager{};

	std::unique_ptr<input::input_manager> input_man{nullptr};

private:

	struct GLFWwindow* glfw_window{nullptr};

	uint16_t frame_width{0}; // vid_reader.height
	uint16_t frame_height{0}; // vid_reader.height

	GLuint video_texture;

	glm::vec2 vertices[4] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

	void render_window_bar();

};