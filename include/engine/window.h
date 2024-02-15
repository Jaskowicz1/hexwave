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

private:

	struct GLFWwindow* glfw_window{nullptr};

	video_manager manager{};
	video_reader vid_reader;

	uint16_t frame_width{0}; // vid_reader.height
	uint16_t frame_height{0}; // vid_reader.height
	uint8_t* frame_data{nullptr}; // = new uint8_t[frame_width * frame_height * 4];

	bool testing_export{false};

	GLuint video_texture;

	GLuint vbo;
	GLuint vao;
	GLuint ibo;
	GLuint shader_program;

	glm::vec2 vertices[4] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

};