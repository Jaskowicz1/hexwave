#define GLFW_INCLUDE_NONE

#include "window.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <fstream>
#include <sstream>

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
	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.TabRounding = 5.f;
	style.FrameRounding = 5.f;
	style.GrabRounding = 5.f;
	style.WindowRounding = 5.f;
	style.PopupRounding = 5.f;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEBUG_OUTPUT);

	// Generate texture
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

window::~window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfw_window);
	glfwTerminate();
}

void window::window_loop() {
	// Allocate frame buffer
	const int frame_width = manager.current_video.width;
	const int frame_height = manager.current_video.width;
	uint8_t* frame_data = new uint8_t[frame_width * frame_height * 4];

	while (!glfwWindowShouldClose(glfw_window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		/*
		 * Need to add a 2D texture here that covers the whole window.
		 * We will then render the video to that texture.
		 */

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					/* do stuff */
				} else if (ImGui::MenuItem("Open", "Ctrl+O")) {
					char filename[1024];
					FILE *f = popen("zenity --file-selection --title=\"Open project\"", "r");
					fgets(filename, 1024, f);

					manager.remove_all_videos();

					std::ifstream project_file(filename);
					std::string file_text;

					while (std::getline (project_file, file_text)) {
						std::stringstream temp_text;
						temp_text << file_text;
						std::string segment;
						std::vector<std::string> seg_list;

						while(std::getline(temp_text, segment, ',')) {
							seg_list.push_back(segment);
						}

						manager.add_video(seg_list[0], seg_list[1], std::stof(seg_list[2]));
					}

					project_file.close();
				} else if (ImGui::MenuItem("Save", "Ctrl+S")) {
					char filename[1024];
					FILE *f = popen("zenity --file-selection --save", "r");
					fgets(filename, 1024, f);

					std::ofstream project_file(filename);

					for(const auto& vid_pair : manager.get_videos()) {
						project_file << vid_pair.second.id << "," << vid_pair.second.name << "," << vid_pair.second.length << "\n";
					}

					project_file.close();
				} else if (ImGui::MenuItem("Quit")) {
					close_window();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Packaging")) {
				if (ImGui::BeginMenu("Export")) {
					if (ImGui::MenuItem("Windows")) {
						/* do stuff */
					} else if (ImGui::MenuItem("Linux")) {
						/* do stuff */
					} else if (ImGui::MenuItem("OSX")) {
						/* do stuff */
					}

					ImGui::EndMenu();
				}

				ImGui::Checkbox("Release Mode", &testing_export);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Play")) {
				if (ImGui::MenuItem("In-Engine")) {
					/* do stuff */
				} else if (ImGui::MenuItem("Standalone")) {
					/* do stuff */
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		manager.render_window();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(glfw_window);
	}
}

void window::close_window() {
	glfwSetWindowShouldClose(glfw_window, true);

};
