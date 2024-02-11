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

window::window() {
	glfwSetErrorCallback([](int error, const char* description) {
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	});

	if (!glfwInit()) {
		std::cout << "glfw failed to initialise." << "\n";
		exit(EXIT_FAILURE);
	}

	glfw_window = glfwCreateWindow(1280, 720, "Hexwave", nullptr, nullptr);

	if (!glfw_window) {
		std::cout << "Failed to create window." << "\n";
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1);

	if (!gladLoaderLoadGL())
		throw std::runtime_error("Error initializing glad");

	glGenTextures(1, &video_texture);
	glBindTexture(GL_TEXTURE_2D, video_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/**
	 * ImGUI stuff
	 */

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

	/**
	 * End of ImGUI stuff
	 */
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
	// /home/archie/Documents/interactive-film-engine/testing/SCENE1.mp4
	// /home/archie/Downloads/tf2teleporter.mp4
	if(!manager.open_video(&vid_reader, "/home/archie/Downloads/tf2teleporter.mp4")) {
		std::cout << "Failed to load video." << "\n";
		return;
	}

	// Allocate frame buffer
	const int frame_width = vid_reader.width;
	const int frame_height = vid_reader.height;
	auto* frame_data = new uint8_t[frame_width * frame_height * 4];

	while (!glfwWindowShouldClose(glfw_window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up orphographic projection
		int window_width, window_height;
		glfwGetWindowSize(glfw_window, &window_width, &window_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);

		int64_t pts;

		if(!manager.read_video_frame(glfw_window, &vid_reader, frame_data, &pts)) {
			std::cout << "Failed to read frame data." << "\n";
			break;
		}

		double pt_in_seconds = pts * (double)vid_reader.time_base.num / (double)vid_reader.time_base.den;
		while (pt_in_seconds > glfwGetTime()) {
			glfwWaitEventsTimeout(pt_in_seconds - glfwGetTime());
		}

		/*
		 * Need to add a 2D texture here that covers the whole window.
		 * We will then render the video to that texture.
		 */

		glBindTexture(GL_TEXTURE_2D, video_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

		// Render whatever you want
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, video_texture);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0); glVertex2i(0, 0);
		glTexCoord2d(1,0); glVertex2i(frame_width, 0);
		glTexCoord2d(1,1); glVertex2i(frame_width, frame_height);
		glTexCoord2d(0,1); glVertex2i(0, frame_height);
		glEnd();
		glDisable(GL_TEXTURE_2D);

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
