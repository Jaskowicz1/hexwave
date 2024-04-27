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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if LIBAVFORMAT_VERSION_MAJOR < 59
#define FFMPEG_LEGACY
#endif

//#define MA_DEBUG_OUTPUT
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	AVAudioFifo* fifo = reinterpret_cast<AVAudioFifo*>(pDevice->pUserData);
	av_audio_fifo_read(fifo, &pOutput, frameCount);

	(void)pInput;
}

window::window() {
	glfwSetErrorCallback([](int error, const char* description) {
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	});

	if (!glfwInit()) {
		std::cout << "glfw failed to initialise." << "\n";
		exit(EXIT_FAILURE);
	}

	glfw_window = glfwCreateWindow(1280, 720, "Hexwave - 0.1", nullptr, nullptr);

	if (!glfw_window) {
		std::cout << "Failed to create window." << "\n";
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1);

	if (!gladLoaderLoadGL())
		throw std::runtime_error("Error initializing glad");

	GLFWimage images[1];
	images[0].pixels = stbi_load("extras/HexwaveIcon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
	glfwSetWindowIcon(glfw_window, 1, images);
	stbi_image_free(images[0].pixels);

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
	video_reader vid_reader{};

	uint8_t* frame_data{};

	ma_device_config device_config{};
	ma_device device{};

	bool started_audio = false;

	while (!glfwWindowShouldClose(glfw_window)) {

		//const auto& render_start = std::chrono::high_resolution_clock::now();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up orphographic projection
		int window_width, window_height;
		glfwGetWindowSize(glfw_window, &window_width, &window_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);

		static bool first_frame = true;

		static bool show_choices;

		if(!manager.current_video.name.empty()) {

			if(first_frame) {
				frame_width = vid_reader.width;
				frame_height = vid_reader.height;

				frame_data = new uint8_t[frame_width * frame_height * 4];

				first_frame = false;
			}

			int64_t pts;

			if(!manager.read_video_frame(glfw_window, &vid_reader, frame_data, &pts)) {
				std::cout << "Failed to read frame data." << "\n";
				break;
			}

			double pt_in_seconds = pts * (double)vid_reader.time_base.num / (double)vid_reader.time_base.den;

			uint64_t pt_rounded = pt_in_seconds;

			if(pts == 0) {
				glfwSetTime(0.0);
				// somehow, this can happen twice. uh oh
				if(frame_data) {
					delete[] frame_data;
					frame_data = nullptr;
				}
				first_frame = true;

				if(device_config.pUserData) {
					ma_device_stop(&device);
					device_config = {};
				}

				device_config = ma_device_config_init(ma_device_type_playback);
				device_config.playback.format   = ma_format_f32;
				#ifndef FFMPEG_LEGACY
					device_config.playback.channels = vid_reader.av_codec_ctx_audio->ch_layout.nb_channels;
				#else
					device_config.playback.channels = vid_reader.av_codec_ctx_audio->channels;
				#endif
				device_config.sampleRate        = vid_reader.av_codec_ctx_audio->sample_rate;
				device_config.dataCallback      = data_callback;
				device_config.pUserData         = vid_reader.av_audio_fifo;

				if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS) {
					printf("Failed to open playback device.\n");
					return;
				}

				if (ma_device_start(&device) != MA_SUCCESS) {
					printf("Failed to start playback device.\n");
					ma_device_uninit(&device);
					return;
				}

				if(manager.current_video.always_show_options) {
					show_choices = true;
				}

			} else if(pt_in_seconds >= manager.current_video.length - 0.75 && !first_frame) {
				std::cout << "End reached, doing next video..." << "\n";
				first_frame = true;
				delete[] frame_data;
				frame_data = nullptr;

				manager.current_video.name = "";

				if(!manager.open_video(&vid_reader, manager.next_video)) {
					std::cout << "Failed to read frame data." << "\n";
					break;
				}
			}

			while (pt_in_seconds > glfwGetTime()) {
				float timeout = pt_in_seconds - glfwGetTime();
				if (timeout >= 0) {
					glfwWaitEventsTimeout(timeout);
				}
			}
		}

		if (frame_data) {
			glBindTexture(GL_TEXTURE_2D, video_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, video_texture);
			// Mac OSX doesn't support this. Why? Who knows.
			glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex2i(0, 0);
			glTexCoord2d(1, 0); glVertex2i(frame_width, 0);
			glTexCoord2d(1, 1); glVertex2i(frame_width, frame_height);
			glTexCoord2d(0, 1); glVertex2i(0, frame_height);
			glEnd();
			glDisable(GL_TEXTURE_2D);
		}

		//const auto& render_finish = std::chrono::high_resolution_clock::now();

		//const auto render_result = std::chrono::duration_cast<std::chrono::milliseconds>(render_finish - render_start);

		//std::cout << "Rendering render time: " << render_result.count() << "ms\n";

		/*
		 * Video renderer finished.
		 */

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();

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

			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Project Settings")) {
					ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Not currently implemented." });
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

		if(!manager.current_video.options.empty() && !first_frame) {
			ImGui::Begin("Options Select", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
			for(const auto& opt : manager.current_video.options) {

				// Pre-cautions
				if(opt.second.id.empty() || opt.second.name.empty()) {
					continue;
				}

				std::string option_text(opt.second.name);
				if(ImGui::Button(option_text.c_str())) {
					first_frame = true;
					delete[] frame_data;
					frame_data = nullptr;

					manager.current_video.name = "";

					std::cout << "attempting to play: " << opt.second.video_id << "\n";
					video vid = manager.get_videos().at(opt.second.video_id);
					std::cout << "video name: " << vid.name << "\n";
					if(!manager.open_video(&vid_reader, vid)) {
						std::cout << "Failed to read frame data." << "\n";
						break;
					}

					break;
				}
				ImGui::SameLine();
			}
			ImGui::End();
		}

		manager.render_window(vid_reader);

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

	delete[] frame_data;
}

void window::close_window() {
	glfwSetWindowShouldClose(glfw_window, true);
};
