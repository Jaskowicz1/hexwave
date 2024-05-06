#pragma once

#include <iostream>
#include "video_manager.h"
#include "glm/vec2.hpp"
#include <glad/gl.h>

class window {

public:
	window();

	~window();

	void window_loop();

	void close_window();

	video_manager manager{};

private:

	struct GLFWwindow* glfw_window{nullptr};

	uint16_t frame_width{0}; // vid_reader.height
	uint16_t frame_height{0}; // vid_reader.height

	bool testing_export{false};

	GLuint video_texture;

	//glm::vec2 vertices[4] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

	GLuint shader_program;
	GLuint video_texture_id;

};