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

	bool testing_export{false};

	GLuint video_texture;

	GLuint vbo;
	GLuint vao;
	GLuint ibo;
	GLuint shader_program;

	glm::vec2 vertices[4] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

};