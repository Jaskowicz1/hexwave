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

	glfw_window = glfwCreateWindow(1280, 720, "Hexwave - 0.2", nullptr, nullptr);

	if (!glfw_window) {
		std::cout << "Failed to create window." << "\n";
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1);

	if (!gladLoaderLoadGL())
		throw std::runtime_error("Error initializing glad");

	std::ifstream icon_file("extras/HexwaveIcon.png");

	if (icon_file.good()) {
		GLFWimage images[1];
		images[0].pixels = stbi_load("extras/HexwaveIcon.png", &images[0].width, &images[0].height, 0, 4); //rgba channels
		glfwSetWindowIcon(glfw_window, 1, images);
		stbi_image_free(images[0].pixels);
	} else {
		std::cout << "Hexwave icon not found! Continuing without." << "\n";
	}

	// Initialise Input Manager
	input_man = std::make_unique<input::input_manager>(glfw_window);

	input_man->on_keyboard_press = [this](int key) {
		if (std::find(input_man->keys_held.begin(), input_man->keys_held.end(), GLFW_KEY_LEFT_CONTROL) != input_man->keys_held.end()) {
			if (std::find(input_man->keys_held.begin(), input_man->keys_held.end(), GLFW_KEY_O) != input_man->keys_held.end()) {
				bool loaded = utilities::load_project(manager, settings);

				if (loaded)
					ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Project loaded successfully!"});
				else
					ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Failed to load project. Check the file is right and project isn't corrupted. If the problem persists, report this issue!"});
			} else if (std::find(input_man->keys_held.begin(), input_man->keys_held.end(), GLFW_KEY_S) != input_man->keys_held.end()) {
				bool saved = utilities::save_project(manager, settings);

				if (saved)
					ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Project saved successfully!"});
				else
					ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Failed to save project. Check the file path you're saving to. If the problem persists, report this issue!"});
			}
		}
	};

	//  glfwSetWindowMonitor

	glGenTextures(1, &video_texture);
	glBindTexture(GL_TEXTURE_2D, video_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	io.IniFilename = nullptr;

	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init("#version 410 core");

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
		std::cerr << "Could not find the FontAwesome font file." << "\n";
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

	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
		std::cerr << "Failed to initialize audio engine." << "\n";
		abort();
	}
}

window::~window() {
	// Free input_man
	input_man.reset();

	ma_engine_uninit(&engine);

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

	int window_width{0};
	int window_height{0};

	int prev_window_width{ 0 };
	int prev_window_height{ 0 };

	ImVec2 uv0 = ImVec2(0.0f, 0.0f);                            // UV coordinates for lower-left
	ImVec2 uv1 = ImVec2(1, 1);

	ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);		// No Background
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);	// No tint

	while (!glfwWindowShouldClose(glfw_window)) {

		static bool resize_required{ false };

		static bool first_frame{ true };
		static bool show_choices{ false };

		static std::string selected_option{};
		static std::string keyboard_hovered_option{};
		static std::string keyboard_selected_option{};

		input_man->input_loop();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up orphographic projection
		glfwGetWindowSize(glfw_window, &window_width, &window_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);

		if (prev_window_width == 0 || prev_window_height == 0) {
			prev_window_width = window_width;
			prev_window_height = window_height;
		} else {
			if (prev_window_width != window_width || prev_window_height != window_height) {
				prev_window_width = window_width;
				prev_window_height = window_height;
				resize_required = true;
			}
		}

		if(!force_video_to_play.empty()) {
			std::cout << "forcing " << force_video_to_play << " to play..." << "\n";
			first_frame = true;

			manager.current_video.name = "";

			//std::cout << "attempting to play: " << opt.second.video_id << "\n";
			video vid = manager.get_videos()[settings.start_id];
			//std::cout << "video name: " << vid.name << "\n";
			if (!manager.open_video(&vid_reader, vid)) {
				std::cout << "Failed to read frame data." << "\n";
				break;
			}

			show_choices = false;
			selected_option = "";

			keyboard_hovered_option = "";
			keyboard_selected_option = "";

			force_video_to_play = "";
		}

		if (!manager.current_video.name.empty()) {

			if (first_frame) {
				frame_width = vid_reader.width;
				frame_height = vid_reader.height;

				if(frame_data) {
					delete[] frame_data;
					frame_data = nullptr;
				}

				frame_data = new uint8_t[frame_width * frame_height * 4];

				first_frame = false;

				selected_option = "";

				keyboard_hovered_option = "";
				keyboard_selected_option = "";

				reload_button_textures();
			} else {
				// NEVER attempt to call resize on first frame, it will already allocate correctly.
				if (resize_required) {
					// We need to free the scaler context as it'll be scaling to the old resolution.
					sws_freeContext(vid_reader.sws_scaler_ctx);
					vid_reader.sws_scaler_ctx = nullptr;

					// Force frame_data to reallocate as screen size has changed.
					if (frame_data) {
						delete[] frame_data;
						frame_data = nullptr;
					}

					// Reallocate frame data next frame.
					// NOTE: This will cause FFmpeg to send a warning about a bad pointer, ignore it, it'll fix next frame.
					first_frame = true;

					// Tell OpenGL the new Viewport size.
					glViewport(0, 0, window_width, window_height);

					if (vid_reader.width < window_width) {
						vid_reader.width = window_width;
					} else {
						vid_reader.width = vid_reader.init_width;
					}

					if (vid_reader.height < window_height) {
						vid_reader.height = window_height;
					} else {
						vid_reader.height = vid_reader.init_height;
					}

					resize_required = false;
				}
			}

			int64_t pts;

			if (!manager.read_video_frame(glfw_window, &vid_reader, frame_data, &pts)) {
				std::cout << "Failed to read frame data." << "\n";
				break;
			}

			double pt_in_seconds = pts * (double)vid_reader.time_base.num / (double)vid_reader.time_base.den;

			if (pts == 0) {
				glfwSetTime(0.0);
				// somehow, this can happen twice. uh oh
				if (frame_data) {
					delete[] frame_data;
					frame_data = nullptr;
				}
				first_frame = true;

				if (device_config.pUserData) {
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

				if (manager.current_video.always_show_options) {
					show_choices = true;
				} else {
					show_choices = false;
				}

			} else if (pt_in_seconds >= manager.current_video.length - 0.75 && !first_frame) {
				std::cout << "End reached, doing next video..." << "\n";
				first_frame = true;
				delete[] frame_data;
				frame_data = nullptr;

				ma_device_uninit(&device);

				manager.current_video.name = "";

				if (!manager.open_video(&vid_reader, manager.next_video)) {
					std::cout << "Failed to read frame data." << "\n";
					break;
				}
			}

			if (manager.current_video.options_show_at > 0) {
				if (pt_in_seconds >= manager.current_video.options_show_at && pt_in_seconds <= manager.current_video.options_hide_at) {
					if (!show_choices) {
						show_choices = true;
					}
				} else if (pt_in_seconds >= manager.current_video.options_show_at && pt_in_seconds >= manager.current_video.options_hide_at) {
					if (show_choices) {
						show_choices = false;
					}
				}
			}

			while (pt_in_seconds > glfwGetTime()) {
				float timeout = pt_in_seconds - glfwGetTime();
				if (timeout >= 0) {
					glfwWaitEventsTimeout(timeout);
				}
			}
		}

		glBindTexture(GL_TEXTURE_2D, video_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, video_texture);
		// Mac OSX doesn't support this. Why? Who knows.
		glBegin(GL_QUADS);
		{
			glTexCoord2d(0, 0); glVertex2i(0, 0);
			glTexCoord2d(1, 0); glVertex2i(frame_width, 0);
			glTexCoord2d(1, 1); glVertex2i(frame_width, frame_height);
			glTexCoord2d(0, 1); glVertex2i(0, frame_height);
		}
		glEnd();
		glDisable(GL_TEXTURE_2D);

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

		render_window_bar();

		if (show_choices) {
			if (!manager.current_video.options.empty() && !first_frame) {
				ImVec2 WindowPos(window_width/2, window_height - 40);
				ImGui::SetNextWindowPos(WindowPos, ImGuiCond_None, ImVec2(0.5f, 0.5f));
				// ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground (This is to prevent the window from having any looks)
				// ImGuiWindowFlags_NoMove (Prevent moving it)
				// ImGuiWindowFlags_AlwaysAutoResize (Auto resize)
				ImGui::Begin("Options Select", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
				for (const auto& opt : manager.current_video.options) {

					// Pre-cautions
					if (opt.second.id.empty() || opt.second.name.empty()) {
						continue;
					}

					if (selected_option.empty()) {
						std::cout << "setting option!" << "\n";
						selected_option = opt.first;
					}

					if (input_man->current_input_type == input::CONTROLLER) {
						if (selected_option == opt.first) {
							ImGui::SetKeyboardFocusHere();
						}
					}

					ImVec2 ButtonCursor = ImGui::GetCurrentWindow()->DC.CursorPos;

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

					std::string option_text(opt.second.name);

					ImTextureID textureId{};
					if (input_man->current_input_type == input::KEYBOARDANDMOUSE) {
						if (!keyboard_selected_option.empty() && keyboard_selected_option == opt.first) {
							textureId = (void*)(intptr_t)pressed_button_texture;
						} else if (!keyboard_hovered_option.empty() && keyboard_hovered_option == opt.first) {
							textureId = (void*)(intptr_t)hovered_button_texture;
						} else {
							textureId = (void*)(intptr_t)button_texture;
						}
					} else {
						if (selected_option == opt.first) {
							textureId = (void*)(intptr_t)hovered_button_texture;
						} else {
							textureId = (void*)(intptr_t)button_texture;
						}
					}

					if(ImGui::ImageButton(("Option_" + opt.first).c_str(), textureId, ImVec2(button_texture_width, button_texture_height), uv0, uv1, bg_col, tint_col)) {
						first_frame = true;
						delete[] frame_data;
						frame_data = nullptr;

						manager.current_video.name = "";

						//std::cout << "attempting to play: " << opt.second.video_id << "\n";
						video vid = manager.get_videos()[opt.second.video_id];
						//std::cout << "video name: " << vid.name << "\n";
						if (!manager.open_video(&vid_reader, vid)) {
							std::cout << "Failed to read frame data." << "\n";
							break;
						}

						show_choices = false;
						selected_option = "";

						keyboard_hovered_option = "";
						keyboard_selected_option = "";

						ImGui::PopStyleVar();

						ma_result res = ma_engine_play_sound(&engine, settings.button_sound_path.c_str(), nullptr);

						if(res == MA_SUCCESS) {
							std::cout << "played sound" << "\n";
						} else {
							std::cout << "sound didn't play! Reason: " << res << "\n";
						}

						ma_device_uninit(&device);

						break;
					}

					ImGui::PopStyleVar();

					if (input_man->current_input_type == input::KEYBOARDANDMOUSE) {
						if(ImGui::IsItemActive()) {
							//std::cout << "clicked" << "\n";
							keyboard_selected_option = opt.first;
						} else if (ImGui::IsItemHovered()) {
							keyboard_hovered_option = opt.first;
						} else {
							if(keyboard_hovered_option == opt.first)
								keyboard_hovered_option = "";

							if(keyboard_selected_option == opt.first)
								keyboard_selected_option = "";
						}
					}

					ImVec2 size = ImVec2(button_texture_width, button_texture_height);

					const ImRect bb(ButtonCursor, ImVec2(ButtonCursor.x + size.x, ButtonCursor.y + size.y));
					ImVec2 text_size = ImGui::CalcTextSize(option_text.c_str());
					ImGuiContext& g = *GImGui;
					const ImGuiStyle& style = g.Style;
					ImGui::RenderTextClipped(bb.Min, bb.Max, option_text.c_str(), NULL, &text_size, style.ButtonTextAlign, &bb);

					/*
					if (input_man->current_input_type == input::CONTROLLER) {
						if (selected_option == opt.first) {
							ImGui::PopStyleColor();
						}
					}
					 */

					ImGui::SameLine();
				}
				ImGui::End();
			}

			input_man->on_controller_button_press = [&](input::controller_inputs key) {
				if (!show_choices) {
					return;
				}

				if (key == input::controller_inputs::CONTROLLER_BUTTON_DOWN) {
					first_frame = true;
					delete[] frame_data;
					frame_data = nullptr;

					manager.current_video.name = "";

					//std::cout << "attempting to play: " << opt.second.video_id << "\n";
					video vid = manager.get_videos()[manager.current_video.options[selected_option].video_id];
					//std::cout << "video name: " << vid.name << "\n";
					if (!manager.open_video(&vid_reader, vid)) {
						std::cout << "Failed to read frame data." << "\n";
					}

					show_choices = false;
					keyboard_hovered_option = "";
					keyboard_selected_option = "";

					ma_device_uninit(&device);

					ma_result res = ma_engine_play_sound(&engine, settings.button_sound_path.c_str(), nullptr);

					if(res == MA_SUCCESS) {
						std::cout << "played sound" << "\n";
					} else {
						std::cout << "sound didn't play! Reason: " << res << "\n";
					}
				} else if (key == input::controller_inputs::CONTROLLER_BUTTON_DPAD_RIGHT) {
					bool found{false};
					for (const auto& opt : manager.current_video.options) {
						if (found) {
							selected_option = opt.first;
							break;
						} else {
							// if the current option is this option, then set "found" to true
							// so the next option can get selected.
							if (selected_option == opt.first) {
								found = true;
							}
						}
					}

					if (!found) {
						for (const auto& opt : manager.current_video.options) {
							selected_option = opt.first;
							break;
						}
					}

				} else if (key == input::controller_inputs::CONTROLLER_BUTTON_DPAD_LEFT) {
					std::string previous_id{};
					for (const auto& opt : manager.current_video.options) {
						if (selected_option == opt.first) {
							if (!previous_id.empty())
								selected_option = previous_id;
							break;
						} else {
							previous_id = opt.first;
						}
					}
				}
			};

			input_man->on_controller_input = [&](input::controller_inputs input){
				if (!show_choices) {
					return;
				}

				if (input == input::INPUT_RIGHT) {
					bool found{false};
					for (const auto& opt : manager.current_video.options) {
						if (found) {
							selected_option = opt.first;
							break;
						} else {
							// if the current option is this option, then set "found" to true
							// so the next option can get selected.
							if (selected_option == opt.first) {
								found = true;
							}
						}
					}

					if (!found) {
						for (const auto& opt : manager.current_video.options) {
							selected_option = opt.first;
							break;
						}
					}

				} else if (input == input::INPUT_LEFT) {
					std::string previous_id{};
					for (const auto& opt : manager.current_video.options) {
						if (selected_option == opt.first) {
							if (!previous_id.empty())
								selected_option = previous_id;
							break;
						} else {
							previous_id = opt.first;
						}
					}
				}
			};
		}

		manager.render_window(vid_reader);
		settings.render_window();

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
}

void window::render_window_bar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Project")) {
				manager.remove_all_videos();

				ImGui::InsertNotification({ImGuiToastType::Success, 3000, "New project started successfully!"});
			} else if (ImGui::MenuItem("Open", "Ctrl+O")) {
				bool loaded = utilities::load_project(manager, settings);

				if (loaded)
					ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Project loaded successfully!"});
				else
					ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Failed to load project. Check the file is right and project isn't corrupted. If the problem persists, report this issue!"});
			} else if (ImGui::MenuItem("Save", "Ctrl+S")) {
				bool saved = utilities::save_project(manager, settings);

				if (saved)
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
				settings.show_window = !settings.show_window;
			}

			ImGui::EndMenu();
		}

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

		if (ImGui::BeginMenu("Play")) {
			if (ImGui::MenuItem("In-Engine")) {
				force_video_to_play = settings.start_id;
			} else if (ImGui::MenuItem("Standalone")) {
				ImGui::InsertNotification({ImGuiToastType::Error, 3000, "Not currently implemented."});
				/* do stuff */
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void window::reload_button_textures() {
	if(!utilities::LoadTextureFromFile(settings.normal_button_path, &button_texture, &button_texture_width, &button_texture_height)) {
		std::cout << "Failed to find button image." << "\n";
		return;
	};

	if(!utilities::LoadTextureFromFile(settings.hovered_button_path, &hovered_button_texture)) {
		std::cout << "Failed to find hovered button image." << "\n";
		return;
	};

	if(!utilities::LoadTextureFromFile(settings.selected_button_path, &pressed_button_texture)) {
		std::cout << "Failed to find pressed button image." << "\n";
		return;
	};
}
