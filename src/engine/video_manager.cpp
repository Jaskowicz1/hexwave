#include <iostream>
#include "video_manager.h"
#include "imgui.h"
#include "imgui_internal.h"

void video_manager::add_video(const video& video_to_add) {
	videos.emplace(video_to_add.id, video_to_add);
}

void video_manager::add_video(const std::string_view id, const std::string_view name, const float length) {
	video vid;
	vid.id = id;
	vid.name = name;
	vid.length = length;
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

static void testing() {
	ImGui::OpenPopup("add_video_popup");
}


void video_manager::render_window() {
	if(ImGui::Begin("Videos")) {
		if (ImGui::Button("Add Video")) {
			testing();
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove ALL Videos")) {
			//remove_all_videos();
		}

		static std::string vid_id_interacting_with;

		if(ImGui::BeginChild("Scrolling")) {

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

			if(!videos.empty()) {
				auto temp_videos = videos;
				for (auto& vid_pair : temp_videos) {
					video& vid = vid_pair.second;
					std::string title("Video: " + vid.id);
					if (ImGui::CollapsingHeader(title.c_str())) {
						// --------------------------------------------------
						ImGui::SeparatorText("Video Information");

						std::string id_txt("Video name: " + vid.name);
						ImGui::Text("%s", id_txt.c_str());
						std::string length_txt("Video length: " + std::to_string(vid.length));
						ImGui::Text("%s", length_txt.c_str());

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
						} else {
							ImGui::Text("There are no options!");
						}

						if (ImGui::Button(std::string("Add Option###" + vid.id).c_str())) {
							vid_id_interacting_with = vid.id;
							ImGui::OpenPopup("add_options_popup");
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Remove All###" + vid.id).c_str())) {
							// Do removal of all options here.
						}

						// --------------------------------------------------
						ImGui::SeparatorText("Video Management");

						if (ImGui::Button(std::string("Play###" + vid.id).c_str())) {
							std::cout << vid.name << "\n";
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Move Up###" + vid.id).c_str())) {
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Move Down###" + vid.id).c_str())) {
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Remove Video###" + vid.id).c_str())) {
							remove_video(vid.id);
						}

						// --------------------------------------------------
						ImGui::SeparatorText("End");
					}
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

			if (ImGui::Button("Add")) {
				if(strlen(id) != 0 && strlen(name) != 0) {
					add_video(id, name, 1.f);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}
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

bool video_manager::open_video(video_reader *state, const char* file) {
	state->av_format_ctx = avformat_alloc_context();
	if(!state->av_format_ctx) {
		std::cout << "Failed to create an AVFormatContext." << "\n";
		return false;
	}

	if(avformat_open_input(&state->av_format_ctx, file, NULL, NULL) != 0) {
		std::cout << "Failed to open video file. Double check the path is correct!" << "\n";
		return false;
	}

	AVCodecParameters* av_codec_params;
	AVCodec* av_codec;
	for (int i = 0; i < state->av_format_ctx->nb_streams; ++i) {
		av_codec_params = state->av_format_ctx->streams[i]->codecpar;
		av_codec = const_cast<AVCodec*>(avcodec_find_decoder(av_codec_params->codec_id));
		if (!av_codec) {
			continue;
		}
		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			state->video_stream_index = i;
			state->width = av_codec_params->width;
			state->height = av_codec_params->height;
			state->time_base = state->av_format_ctx->streams[i]->time_base;
			break;
		}
	}

	if (state->video_stream_index == -1) {
		printf("Couldn't find valid video stream inside file\n");
		return false;
	}

	// Set up a codec context for the decoder
	state->av_codec_ctx = avcodec_alloc_context3(av_codec);
	if (!state->av_codec_ctx) {
		printf("Couldn't create AVCodecContext\n");
		return false;
	}
	if (avcodec_parameters_to_context(state->av_codec_ctx, av_codec_params) < 0) {
		printf("Couldn't initialize AVCodecContext\n");
		return false;
	}
	if (avcodec_open2(state->av_codec_ctx, av_codec, NULL) < 0) {
		printf("Couldn't open codec\n");
		return false;
	}

	state->av_frame = av_frame_alloc();
	if (!state->av_frame) {
		printf("Couldn't allocate AVFrame\n");
		return false;
	}

	state->av_packet = av_packet_alloc();
	if (!state->av_packet) {
		printf("Couldn't allocate AVPacket\n");
		return false;
	}

	return true;
}

bool video_manager::read_video_frame(video_reader *state, uint8_t *frame_buffer) {
	int response{-1};

	while(av_read_frame(state->av_format_ctx, state->av_packet) >= 0) {
		if(state->av_packet->stream_index != state->video_stream_index) {
			av_packet_unref(state->av_packet);
			continue;
		}

		response = avcodec_send_packet(state->av_codec_ctx, state->av_packet);
		if(response < 0) {
			std::cout << "Failed to decode packet: ";
			return false;
		}

		response = avcodec_receive_frame(state->av_codec_ctx, state->av_frame);

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

	// Set up sws scaler
	if (!state->sws_scaler_ctx) {
		//auto source_pix_fmt = correct_for_deprecated_pixel_format(state->av_codec_ctx->pix_fmt);
		state->sws_scaler_ctx = sws_getContext(state->width, state->height, state->av_codec_ctx->pix_fmt,
						       state->width, state->height, AV_PIX_FMT_RGB0,
						SWS_BILINEAR, NULL, NULL, NULL);
	}
	if (!state->sws_scaler_ctx) {
		printf("Couldn't initialize sw scaler\n");
		return false;
	}

	uint8_t* dest[4] = { frame_buffer, NULL, NULL, NULL };
	int dest_linesize[4] = { state->width * 4, 0, 0, 0 };
	sws_scale(state->sws_scaler_ctx, state->av_frame->data, state->av_frame->linesize, 0, state->av_frame->height, dest, dest_linesize);

	return true;
}
