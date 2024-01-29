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

// ffmpeg moment
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

const char *vertexShaderSrc = R"(
#version 450

layout (location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos, 1, 1);
}

)";

const char *fragmentShaderSrc = R"(
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

	/**
	 * Compile shader
	 */
	int success;
	char infoLog[512];
	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSrc, 0);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cerr << "Vertex shader compilation failed:" << std::endl;
		std::cerr << infoLog << std::endl;
	}

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSrc, 0);
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
	GLuint vao;
	glCreateVertexArrays(1, &vao);

	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE,
				  offsetof(glm::vec2, x));

	glVertexArrayAttribBinding(vao, 0, 0);

	glm::vec2 vertices[] = {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

	GLuint vbo;
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(glm::vec2) * 4, vertices,
			     GL_DYNAMIC_STORAGE_BIT);

	std::uint32_t indices[] = {0, 2, 1, 2, 0, 3};

	GLuint ibo;
	glCreateBuffers(1, &ibo);
	glNamedBufferStorage(ibo, sizeof(std::uint32_t) * 6, indices,
			     GL_DYNAMIC_STORAGE_BIT);

	glBindVertexArray(vao);
	glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec2));
	glVertexArrayElementBuffer(vao, ibo);
	glUseProgram(program);
	glClearColor(1, 1, 1, 1);
}

window::~window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfw_window);
	glfwTerminate();
}

void window::window_loop() {
	AVFormatContext* av_format_ctx = avformat_alloc_context();
	int width, height;

	if (!av_format_ctx) {
		printf("Couldn't created AVFormatContext\n");
	}

	if (avformat_open_input(&av_format_ctx, "../testing/SCENE1.mp4", nullptr, nullptr) != 0) {
		printf("Couldn't open video file\n");
	}

	// Find the first valid video stream inside the file
	int video_stream_index = -1;
	AVCodecParameters* av_codec_params;
	const AVCodec* av_codec;
	AVRational time_base;
	for (int i = 0; i < av_format_ctx->nb_streams; ++i) {
		av_codec_params = av_format_ctx->streams[i]->codecpar;
		av_codec = avcodec_find_decoder(av_codec_params->codec_id);
		if (!av_codec) {
			continue;
		}
		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			width = av_codec_params->width;
			height = av_codec_params->height;
			time_base = av_format_ctx->streams[i]->time_base;
			break;
		}
	}
	if (video_stream_index == -1) {
		printf("Couldn't find valid video stream inside file\n");
	}

	// Set up a codec context for the decoder
	AVCodecContext* av_codec_ctx = avcodec_alloc_context3(av_codec);
	if (!av_codec_ctx) {
		printf("Couldn't create AVCodecContext\n");
	}
	if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
		printf("Couldn't initialize AVCodecContext\n");
	}
	if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
		printf("Couldn't open codec\n");
	}

	AVFrame* av_frame = av_frame_alloc();
	if (!av_frame) {
		printf("Couldn't allocate AVFrame\n");
	}

	AVPacket* av_packet = av_packet_alloc();
	if (!av_packet) {
		printf("Couldn't allocate AVPacket\n");
	}

	// Allocate frame buffer
	const int frame_width = 400;
	const int frame_height = 400;
	uint8_t* frame_data = new uint8_t[frame_width * frame_height * 4];

	while (!glfwWindowShouldClose(glfw_window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		int response;

		while (av_read_frame(av_format_ctx, av_packet) >= 0) {
			if (av_packet->stream_index != video_stream_index) {
				av_packet_unref(av_packet);
				continue;
			}

			response = avcodec_send_packet(av_codec_ctx, av_packet);
			if (response < 0) {
				//printf("Failed to decode packet: %s\n", av_err2str(response));
			}

			response = avcodec_receive_frame(av_codec_ctx, av_frame);
			if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				av_packet_unref(av_packet);
				continue;
			} else if (response < 0) {
				//printf("Failed to decode packet: %s\n", av_err2str(response));
			}

			av_packet_unref(av_packet);
			break;
		}

		// Set up sws scaler
		if (!manager.current_video.sws_scaler_ctx) {
			manager.current_video.sws_scaler_ctx = sws_getContext(width, height, av_codec_ctx->pix_fmt,
							width, height, AV_PIX_FMT_RGB0,
							SWS_BILINEAR, NULL, NULL, NULL);
		}
		if (!manager.current_video.sws_scaler_ctx) {
			printf("Couldn't initialize sw scaler\n");
		}

		//uint8_t* dest[4] = { frame_buffer, NULL, NULL, NULL };
		//int dest_linesize[4] = { width * 4, 0, 0, 0 };
		//sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);

		glGenTextures(1, &test_texture);
		glBindTexture(GL_TEXTURE_2D, test_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, test_texture);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data
		);

		glActiveTexture(GL_TEXTURE);
		glBindTexture(GL_TEXTURE_2D, test_texture);

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
					manager.remove_all_videos();
				} else if (ImGui::MenuItem("Open", "Ctrl+O")) {
					utilities::load_project(manager);
				} else if (ImGui::MenuItem("Save", "Ctrl+S")) {
					utilities::save_project(manager);
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
