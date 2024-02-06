#define GLFW_INCLUDE_NONE

#include "window.h"
#include "utilities/file_management.h"
#include "glm/vec2.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <fstream>
#include <sstream>

#include "ImGuiNotify.hpp"
#include "IconsFontAwesome6.h"

// ffmpeg moment
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

const char *vertex_shader_src = R"(
#version 450
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 1, 1);
}
)";

const char *fragment_shader_src = R"(
#version 450
layout (location = 0) out vec4 outColor;
void main() {
    outColor = vec4(1, 0, 0, 1);
}
)";

window::window() {

	// Just do a lambda here because static is nothing but ugly.
	glfwSetErrorCallback([](int error, const char* description) {
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	});

	if (!glfwInit()) {
		std::cout << "glfw failed to initialise." << "\n";
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfw_window = glfwCreateWindow(1280, 720, "Interactive Film Engine", nullptr, nullptr);

	if (!glfw_window) {
		std::cout << "Failed to create window." << "\n";
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1);

	if (!gladLoaderLoadGL())
		throw std::runtime_error("Error initializing glad");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");

	ImGui::StyleColorsDark();

	/*
	 * Start of notification system loading
	 */

	io.Fonts->AddFontDefault();

	float baseFontSize = 16.0f; // Default font size
	float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

	// Check if FONT_ICON_FILE_NAME_FAS is a valid path
	std::ifstream fontAwesomeFile(FONT_ICON_FILE_NAME_FAS);

	if (!fontAwesomeFile.good()) {
		// If it's not good, then we can't find the font and should abort
		std::cerr << "Could not find the FontAwesome font file." << std::endl;
		abort();
	}

	static const ImWchar iconsRanges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	iconsConfig.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, iconFontSize, &iconsConfig, iconsRanges);

	/*
	 * End of notification system loading
	 */

	// Make borders round on ImGui.
	ImGuiStyle& style = ImGui::GetStyle();
	style.TabRounding = 5.f;
	style.FrameRounding = 5.f;
	style.GrabRounding = 5.f;
	style.WindowRounding = 5.f;
	style.PopupRounding = 5.f;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEBUG_OUTPUT);

	/*
	 * Compile shader
	 */
	int success;
	char infoLog[512];
	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_shader_src, 0);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cerr << "Vertex shader compilation failed:" << std::endl;
		std::cerr << infoLog << std::endl;
	}

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragment_shader_src, 0);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cerr << "Fragment shader compilation failed:" << std::endl;
		std::cerr << infoLog << std::endl;
	}

	auto program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "Shader linking failed:" << std::endl;
		std::cerr << infoLog << std::endl;
	}

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);

	/**
	 * Create vertex array and buffers
	 */
	glCreateVertexArrays(1, &vao);

	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE,
				  offsetof(glm::vec2, x));

	glVertexArrayAttribBinding(vao, 0, 0);

	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(glm::vec2) * 4, vertices,
			     GL_DYNAMIC_STORAGE_BIT);

	std::uint32_t indices[] = {0, 2, 1, 2, 0, 3};

	glCreateBuffers(1, &ibo);
	glNamedBufferStorage(ibo, sizeof(std::uint32_t) * 6, indices,
			     GL_DYNAMIC_STORAGE_BIT);

	glBindVertexArray(vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec2));
	glVertexArrayElementBuffer(vao, ibo);
	glUseProgram(program);

}

window::~window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfw_window);
	glfwTerminate();
}

void window::window_loop() {

	video_reader vid_reader;
	if(!manager.open_video(&vid_reader, "/home/archie/Downloads/tf2teleporter.mp4")) {
		std::cout << "Failed to load video." << "\n";
		return;
	}

	glGenTextures(1, &video_texture);
	glBindTexture(GL_TEXTURE_2D, video_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Allocate frame buffer
	const int frame_width = vid_reader.width;
	const int frame_height = vid_reader.height;
	auto* frame_data = new uint8_t[frame_width * frame_height * 4];

	while (!glfwWindowShouldClose(glfw_window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(!manager.read_video_frame(&vid_reader, frame_data)) {
			std::cout << "Failed to read frame data." << "\n";
			break;
		}

		/*
		 * Need to add a 2D texture here that covers the whole window.
		 * We will then render the video to that texture.
		 */

		glBindTexture(GL_TEXTURE_2D, video_texture);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data
		);

		glActiveTexture(GL_TEXTURE);
		glBindTexture(GL_TEXTURE_2D, video_texture);
		glBindVertexArray(vao);
		glDrawElements(GL_QUADS, sizeof(vertices), GL_UNSIGNED_INT, 0);

		/*
		 * Video renderer finished.
		 */

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					manager.remove_all_videos();

					ImGui::InsertNotification({ImGuiToastType::Success, 3000, "New project started successfully!"});
				} else if (ImGui::MenuItem("Open", "Ctrl+O")) {
					bool loaded = utilities::load_project(manager);

					if(loaded)
						ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Project loaded successfully!"});
					else
						ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Failed to load project. Check the file is right and project isn't corrupted. If the problem persists, report this issue!"});
				} else if (ImGui::MenuItem("Save", "Ctrl+S")) {
					bool saved = utilities::save_project(manager);

					if(saved)
						ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Project saved successfully!"});
					else
						ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Failed to save project. Check the file path you're saving to. If the problem persists, report this issue!"});
				} else if (ImGui::MenuItem("Quit")) {
					close_window();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Packaging")) {
				if (ImGui::BeginMenu("Export")) {
					if (ImGui::MenuItem("Windows")) {
						ImGui::InsertNotification({ImGuiToastType::Error, 3000, "Not currently implemented."});
						/* do stuff */
					} else if (ImGui::MenuItem("Linux")) {
						ImGui::InsertNotification({ImGuiToastType::Error, 3000, "Not currently implemented."});
						/* do stuff */
					} else if (ImGui::MenuItem("OSX")) {
						ImGui::InsertNotification({ImGuiToastType::Error, 3000, "Not currently implemented."});
						/* do stuff */
					}

					ImGui::EndMenu();
				}

				ImGui::Checkbox("Release Mode", &testing_export);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Play")) {
				if (ImGui::MenuItem("In-Engine")) {
					ImGui::InsertNotification({ImGuiToastType::Error, 3000, "Not currently implemented."});
					/* do stuff */
				} else if (ImGui::MenuItem("Standalone")) {
					ImGui::InsertNotification({ImGuiToastType::Error, 3000, "Not currently implemented."});
					/* do stuff */
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		manager.render_window();

		/**
		 * Notifications Rendering Start
		 */

		// Notifications style setup
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Disable round borders
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders

		// Notifications color setup
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color

		// Main rendering function
		ImGui::RenderNotifications();

		//——————————————————————————————— WARNING ———————————————————————————————
		// Argument MUST match the amount of ImGui::PushStyleVar() calls
		ImGui::PopStyleVar(2);
		// Argument MUST match the amount of ImGui::PushStyleColor() calls
		ImGui::PopStyleColor(1);

		/**
		 * Notifications Rendering End
		*/

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(glfw_window);
		glfwPollEvents();
	}
}

void window::close_window() {
	glfwSetWindowShouldClose(glfw_window, true);
};
