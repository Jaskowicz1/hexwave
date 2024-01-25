#define GLFW_INCLUDE_NONE

#include "window.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

		// Render whatever you want
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		float vertices[] = { // X, Y
			0.f, 0.f,
			0.87f, -0.5f,
			-0.87f, -0.5f,
		};

		glVertexPointer(2, GL_FLOAT, sizeof (float) * 2, vertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0); glVertex2i(200, 200);
		glTexCoord2d(1,0); glVertex2i(200 + frame_width, 200);
		glTexCoord2d(1,1); glVertex2i(200 + frame_width, 200 + frame_height);
		glTexCoord2d(0,1); glVertex2i(200, 200 + frame_height);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Project")) {
					/* do stuff */
				} else if (ImGui::MenuItem("Open", "Ctrl+O")) {
					/* do stuff */
				} else if (ImGui::MenuItem("Save", "Ctrl+S")) {
					/* do stuff */
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
