#include <iostream>
#include <fstream>
#include "video_manager.h"
#include "utilities/file_management.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiNotify.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#if LIBAVFORMAT_VERSION_MAJOR < 59
	#ifndef FFMPEG_LEGACY
	#define FFMPEG_LEGACY
	#endif
#endif

void video_manager::add_video(const video& video_to_add) {
	videos.emplace(video_to_add.id, video_to_add);
}

void video_manager::add_video(const json& j) {
	video vid;
	vid.fill_from_json(&j);
	videos.emplace(vid.id, vid);
}

void video_manager::add_video(const std::string_view id, const std::string_view name, const double length, const std::string_view path) {
	video vid;
	vid.id = id;
	vid.name = name;
	vid.length = length;
	vid.path = path;
	videos.emplace(id, vid);
}

void video_manager::remove_video(const std::string_view id) {
	if(videos.find(std::string(id.begin(), id.end())) != videos.end()) {
		videos.erase(std::string(id.begin(), id.end()));
	}
}

void video_manager::remove_all_videos() {
	videos.clear();
}

void video_manager::add_option(video& vid, const std::string_view id, const std::string_view name, const std::string_view video_id) {
	option opt;
	opt.id = id;
	opt.name = name;
	opt.video_id = video_id;
	vid.options.emplace(id, opt);
}


void video_manager::render_window(video_reader& reader) {
	if(ImGui::Begin("Videos")) {
		if (ImGui::Button("Add Video")) {
			ImGui::OpenPopup("add_video_popup");
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove ALL Videos")) {
			remove_all_videos();
		}

		static std::string vid_id_interacting_with;

		if(ImGui::BeginChild("Scrolling")) {
			if(!videos.empty()) {
				auto temp_videos = videos;
				for (auto& vid_pair : temp_videos) {
					video& vid = vid_pair.second;
					ImGui::PushID(vid.id.c_str());
					std::string title("Video: " + vid.id);
					if (ImGui::CollapsingHeader(title.c_str())) {
						// --------------------------------------------------
						ImGui::SeparatorText("Video Information");

						std::string id_txt("Video name: " + vid.name);
						ImGui::Text("%s", id_txt.c_str());

						std::string length_txt("Video length: " + std::to_string(vid.length) + " seconds");
						ImGui::Text("%s", length_txt.c_str());

						std::string linked_txt("Video linked to: " + vid.next_video_id);
						ImGui::Text("%s", linked_txt.c_str());

						std::string loop_txt("Video loops? " + std::string(vid.loop ? "True" : "False"));
						ImGui::Text("%s", loop_txt.c_str());

						std::string path_txt("Video path: " + vid.path);
						ImGui::Text("%s", path_txt.c_str());

						// --------------------------------------------------

						ImGui::SeparatorText("Options Information");

						if(!vid.options.empty()) {
							if (ImGui::CollapsingHeader("Options")) {
								for (auto& opt_pair : vid.options) {
									option& opt = opt_pair.second;
									std::string opt_title("Option: " + opt.id);
									if (ImGui::TreeNode(opt_title.c_str())) {
										// --------------------------------------------------
										ImGui::SeparatorText("Option Information");

										std::string opt_id("Option ID: " + opt.id);
										ImGui::Text("%s", opt_id.c_str());

										std::string opt_txt("Option name: " + opt.name);
										ImGui::Text("%s", opt_txt.c_str());

										std::string opt_video_id("Option linked to video ID: " + opt.video_id);
										ImGui::Text("%s", opt_video_id.c_str());

										// --------------------------------------------------
										ImGui::SeparatorText("End");

										ImGui::TreePop();
									}
								}
							}

							std::string opts_show("Options always show? " + std::string(vid.always_show_options ? "True" : "False"));
							ImGui::Text("%s", opts_show.c_str());

							std::string opts_show_at("Options show at: " + std::to_string(vid.options_show_at));
							ImGui::Text("%s", opts_show_at.c_str());

							std::string opts_hide_at("Options hide at: " + std::to_string(vid.options_hide_at));
							ImGui::Text("%s", opts_hide_at.c_str());
						} else {
							ImGui::Text("There are no options!");
						}

						if (ImGui::Button(std::string("Add Option##" + vid.id).c_str())) {
							vid_id_interacting_with = vid.id;
							ImGui::OpenPopup("add_options_popup");
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Remove All##" + vid.id).c_str())) {
							// Do removal of all options here.
						}

						if (ImGui::BeginPopup("add_options_popup")) {
							static char id[64];
							ImGui::InputText("ID", id, IM_ARRAYSIZE(id));

							static char name[64];
							ImGui::InputText("Name", name, IM_ARRAYSIZE(name));

							static char video_id[64];
							ImGui::InputText("Option linked to video ID", video_id, IM_ARRAYSIZE(video_id));

							if (ImGui::Button("Add")) {
								if(strlen(id) != 0 && strlen(name) != 0 && strlen(video_id) != 0) {
									add_option(videos[vid_id_interacting_with], id, name, video_id);
									ImGui::CloseCurrentPopup();
									vid_id_interacting_with = "";
								}
							}

							ImGui::EndPopup();
						}

						// --------------------------------------------------
						ImGui::SeparatorText("Video Management");

						if (ImGui::Button(std::string("Play##" + vid.id).c_str())) {
							if(!open_video(&reader, vid)) {
								ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Failed to load video."});
								return;
							}

							ImGui::InsertNotification({ImGuiToastType::Success, 3000, "Playing video!"});
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Move Up##" + vid.id).c_str())) {
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Move Down##" + vid.id).c_str())) {
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Remove Video##" + vid.id).c_str())) {
							remove_video(vid.id);
						}

						// --------------------------------------------------
						ImGui::SeparatorText("End");
					}
					ImGui::PopID();
				}
			} else {
				ImGui::Text("There are no videos!");
			}

			ImGui::EndChild();
		}

		if (ImGui::BeginPopup("add_video_popup")) {
			static char id[64];
			ImGui::InputText("ID", id, IM_ARRAYSIZE(id));

			static char name[64];
			ImGui::InputText("Name", name, IM_ARRAYSIZE(name));

			static char linked_id[512];
			ImGui::InputText("Linked to", linked_id, IM_ARRAYSIZE(linked_id));

			static bool should_loop{ false };
			ImGui::Checkbox("Loop?", &should_loop);

			static bool always_show_options{ false };
			ImGui::Checkbox("Always Display Options?", &always_show_options);

			static int options_show_at{ 0 };
			ImGui::InputInt("Options Show At: ", &options_show_at);

			static int options_hide_at{ 0 };
			ImGui::InputInt("Options Hide At: ", &options_hide_at);

			static char path[1024];
			ImGui::InputText("Video Path", path, IM_ARRAYSIZE(path));
			ImGui::SameLine();

			if (ImGui::Button(std::string("Select Video").c_str())) {
				std::string video_path{utilities::get_file_from_prompt(false, "Open Video File", "*.mp4 | *.mov | *.wmv | *.avi | *.mkv | *.webm | *.*", "MP4\0*.mp4\0MOV\0*.mov\0WMV\0*.wmv\0AVI\0*.avi\0MKV\0*.mkv\0WEBM\0*.webm\0All\0*.*\0")};

				std::ifstream video_file(video_path);

				if (video_file.good()) {
					const char* temp_path = video_path.c_str();
					// Copy temp path to path if the path was valid.
					std::memcpy(path, temp_path, std::strlen(temp_path));
				} else {
					ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Failed to open video file, please make sure it's a valid video!" });
				}
			}

			if (ImGui::Button("Add")) {
				if(strlen(id) != 0 && strlen(name) != 0 && strlen(linked_id) != 0 && strlen(path) != 0) {
					double len = get_video_length(path);
					if(len != 0) {
						add_video(id, name, len, path);
						videos[id].next_video_id = linked_id;
						videos[id].loop = should_loop;
						videos[id].always_show_options = always_show_options;
						videos[id].options_show_at = options_show_at;
						videos[id].options_hide_at = options_hide_at;
						ImGui::CloseCurrentPopup();
					} else {
						ImGui::InsertNotification({ImGuiToastType::Error, 5000, "Invalid path! Check the path of the video and make sure it's the right file type!"});
					}
				} else {
					ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Failed to add video! Ensure that ID, name, linked_id, and path, are not empty!" });
				}
			}

			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

std::map<std::string, video> &video_manager::get_videos() {
	return videos;
}

void video_manager::update_option(video &vid, option &opt) {
	auto x = vid.options.find(opt.id);
	if(x != vid.options.end()) {
		x->second.name = opt.name;
		x->second.video_id = opt.video_id;
	}
}

double video_manager::get_video_length(const char *file) {
	AVFormatContext* av_format_ctx = avformat_alloc_context();
	if(!av_format_ctx) {
		std::cout << "Failed to create an AVFormatContext." << "\n";
		return 0;
	}

	if(avformat_open_input(&av_format_ctx, file, NULL, NULL) != 0) {
		std::cout << "Failed to open video file. Double check the path is correct!" << "\n";
		return 0;
	}

	AVCodecParameters* av_codec_params{nullptr};
	AVCodec* av_codec{nullptr};
	AVRational time_base{};

	double duration{0};

	for (int i = 0; i < av_format_ctx->nb_streams; ++i) {
		av_codec_params = av_format_ctx->streams[i]->codecpar;
		av_codec = const_cast<AVCodec*>(avcodec_find_decoder(av_codec_params->codec_id));
		if (!av_codec) {
			continue;
		}
		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			duration = av_format_ctx->streams[i]->duration;
			time_base = av_format_ctx->streams[i]->time_base;
			break;
		}
	}

	avformat_close_input(&av_format_ctx);
	avformat_free_context(av_format_ctx);

	return duration * (double)time_base.num / (double)time_base.den;
}

bool video_manager::open_video(video_reader *state, const video& vid) {
	sws_freeContext(state->sws_scaler_ctx);
	swr_free(&state->swr_resampler_ctx);

	state->sws_scaler_ctx = nullptr;
	state->swr_resampler_ctx = nullptr;

	avformat_close_input(&state->av_format_ctx);
	avformat_free_context(state->av_format_ctx);

	avcodec_close(state->av_codec_ctx_video);
	avcodec_free_context(&state->av_codec_ctx_video);

	avcodec_close(state->av_codec_ctx_audio);
	avcodec_free_context(&state->av_codec_ctx_audio);

	av_frame_free(&state->av_frame);
	av_frame_free(&state->av_audio_frame);
	av_packet_free(&state->av_packet);

	const char* file = vid.path.c_str();

	state->av_format_ctx = avformat_alloc_context();
	if(!state->av_format_ctx) {
		std::cout << "Failed to create an AVFormatContext." << "\n";
		return false;
	}

	if(avformat_open_input(&state->av_format_ctx, file, NULL, NULL) != 0) {
		std::cout << "Failed to open video file. Double check the path is correct!" << "\n";
		return false;
	}

	AVCodecParameters* av_codec_video_params{nullptr};
	AVCodecParameters* av_codec_audio_params{nullptr};

	AVCodec* av_codec_video{nullptr};
	AVCodec* av_codec_audio{nullptr};

	for (int i = 0; i < state->av_format_ctx->nb_streams; ++i) {

		if (state->av_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			av_codec_video_params = state->av_format_ctx->streams[i]->codecpar;
			av_codec_video = const_cast<AVCodec*>(avcodec_find_decoder(av_codec_video_params->codec_id));

			state->video_stream_index = i;
			state->width = av_codec_video_params->width;
			state->init_width = av_codec_video_params->width;
			state->height = av_codec_video_params->height;
			state->init_height = av_codec_video_params->height;
			state->time_base = state->av_format_ctx->streams[i]->time_base;
		} else if (state->av_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			av_codec_audio_params = state->av_format_ctx->streams[i]->codecpar;
			av_codec_audio = const_cast<AVCodec*>(avcodec_find_decoder(av_codec_audio_params->codec_id));

			state->audio_stream_index = i;
		}
	}

	if (state->video_stream_index == -1) {
		printf("Couldn't find valid video stream inside file\n");
		return false;
	}

	// Set up a codec context for the decoder
	state->av_codec_ctx_video = avcodec_alloc_context3(av_codec_video);
	if (!state->av_codec_ctx_video) {
		printf("Couldn't create AVCodecContext for video.\n");
		return false;
	}
	if (avcodec_parameters_to_context(state->av_codec_ctx_video, av_codec_video_params) < 0) {
		printf("Couldn't initialize AVCodecContext for video.\n");
		return false;
	}

	state->av_codec_ctx_video->thread_count = 0;

	if (av_codec_video->capabilities & AV_CODEC_CAP_FRAME_THREADS)
		state->av_codec_ctx_video->thread_type = FF_THREAD_FRAME;
	else if (av_codec_video->capabilities & AV_CODEC_CAP_SLICE_THREADS)
		state->av_codec_ctx_video->thread_type = FF_THREAD_SLICE;
	else
		state->av_codec_ctx_video->thread_count = 1;

	if (avcodec_open2(state->av_codec_ctx_video, av_codec_video, NULL) < 0) {
		printf("Couldn't open video codec\n");
		return false;
	}

	state->av_codec_ctx_audio = avcodec_alloc_context3(av_codec_audio);
	if (!state->av_codec_ctx_audio) {
		printf("Couldn't create AVCodecContext for audio.\n");
		return false;
	}
	if (avcodec_parameters_to_context(state->av_codec_ctx_audio, av_codec_audio_params) < 0) {
		printf("Couldn't initialize AVCodecContext for audio.\n");
		return false;
	}

	state->av_codec_ctx_audio->thread_count = 0;

	if (av_codec_audio->capabilities & AV_CODEC_CAP_FRAME_THREADS)
		state->av_codec_ctx_audio->thread_type = FF_THREAD_FRAME;
	else if (av_codec_audio->capabilities & AV_CODEC_CAP_SLICE_THREADS)
		state->av_codec_ctx_audio->thread_type = FF_THREAD_SLICE;
	else
		state->av_codec_ctx_audio->thread_count = 1;

	if (avcodec_open2(state->av_codec_ctx_audio, av_codec_audio, NULL) < 0) {
		printf("Couldn't open audio codec\n");
		return false;
	}

	state->av_frame = av_frame_alloc();
	if (!state->av_frame) {
		printf("Couldn't allocate AVFrame\n");
		return false;
	}

	state->av_audio_frame = av_frame_alloc();
	if (!state->av_audio_frame) {
		printf("Couldn't allocate audio AVFrame\n");
		return false;
	}

	state->av_packet = av_packet_alloc();
	if (!state->av_packet) {
		printf("Couldn't allocate AVPacket\n");
		return false;
	}

	//std::cout << "sample rate: " << av_codec_audio_params->sample_rate << "\n";

	// This does not work in FFmpeg 4.4.x and below. Need to wrap in legacy ifdef eventually.
	swr_alloc_set_opts2(&state->swr_resampler_ctx,
			    &av_codec_audio_params->ch_layout, AV_SAMPLE_FMT_FLT, av_codec_audio_params->sample_rate,
			    &av_codec_audio_params->ch_layout, (AVSampleFormat)av_codec_audio_params->format, av_codec_audio_params->sample_rate,
			    0, nullptr);

#ifndef FFMPEG_LEGACY
	state->av_audio_fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, av_codec_audio_params->ch_layout.nb_channels, 1);
#else
	state->av_audio_fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, av_codec_audio_params->channels, 1);
#endif

	current_video = vid;

	if(vid.loop) {
		next_video = current_video;
	} else if(!vid.next_video_id.empty()) {
		next_video = videos[vid.next_video_id];
	} else {
		bool found = false;

		for(const auto& temp_vid : videos) {
			if(temp_vid.second.id == current_video.id) {
				found = true;
				continue;
			}

			if(found) {
				next_video = temp_vid.second;
				break;
			}
		}
	}

	return true;
}

bool video_manager::read_video_frame(GLFWwindow* window, video_reader* state, uint8_t* frame_buffer, int64_t* pts) {
	int response{-1};

	while(av_read_frame(state->av_format_ctx, state->av_packet) >= 0) {
		if(state->av_packet->stream_index != state->video_stream_index && state->av_packet->stream_index != state->audio_stream_index) {
			//std::cout << "unref..." << "\n";
			av_packet_unref(state->av_packet);
			continue;
		}

		if(state->av_packet->stream_index == state->audio_stream_index) {
			response = avcodec_send_packet(state->av_codec_ctx_audio, state->av_packet);
			if(response < 0) {
				std::cout << "Failed to decode packet: ";
				return false;
			}

			response = avcodec_receive_frame(state->av_codec_ctx_audio, state->av_audio_frame);

			if(response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				std::cout << "bad response..." << "\n";
				continue;
			}

			AVFrame* temp_frame = av_frame_alloc();
			temp_frame->sample_rate = state->av_audio_frame->sample_rate;
#ifndef FFMPEG_LEGACY
			temp_frame->ch_layout = state->av_audio_frame->ch_layout;
#else
			temp_frame->channel_layout = state->av_audio_frame->channel_layout;
			temp_frame->channels = state->av_audio_frame->channels;
#endif
			temp_frame->format = AV_SAMPLE_FMT_FLT;

			response = swr_convert_frame(state->swr_resampler_ctx, temp_frame, state->av_audio_frame);
			av_frame_unref(state->av_audio_frame);
			av_audio_fifo_write(state->av_audio_fifo, (void**)temp_frame->data, temp_frame->nb_samples);
			av_frame_free(&temp_frame);

			continue;
		}

		response = avcodec_send_packet(state->av_codec_ctx_video, state->av_packet);
		if(response < 0) {
			std::cout << "Failed to decode packet: ";
			return false;
		}

		response = avcodec_receive_frame(state->av_codec_ctx_video, state->av_frame);

		if(response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			av_packet_unref(state->av_packet);
			continue;
		} else if(response < 0) {
			std::cout << "Failed to decode packet: ";
			return false;
		}

		av_packet_unref(state->av_packet);
		break;
	}

	*pts = state->av_frame->pts;

	// Set up sws scaler
	if (!state->sws_scaler_ctx) {
		int window_width;
		int window_height;
		glfwGetWindowSize(window, &window_width, &window_height);
		state->sws_scaler_ctx = sws_getContext(state->width, state->height, state->av_codec_ctx_video->pix_fmt,
						       window_width, window_height, AV_PIX_FMT_RGB0,
						       SWS_BICUBIC, NULL, NULL, NULL);
	}
	if (!state->sws_scaler_ctx) {
		printf("Couldn't initialize sw scaler\n");
		return false;
	}

	uint8_t* dest[4] = { frame_buffer, NULL, NULL, NULL };
	int dest_linesize[4] = { state->width * 4, 0, 0, 0 };
	sws_scale(state->sws_scaler_ctx, state->av_frame->data, state->av_frame->linesize, 0, state->height, dest, dest_linesize);

	return true;
}
