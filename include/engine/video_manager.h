#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <map>
#include "video.h"
#include "GLFW/glfw3.h"

// ffmpeg moment
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

struct video_reader {
	int width{0};
	int height{0};
	AVRational time_base;
	AVFormatContext* av_format_ctx{nullptr};
	AVCodecContext* av_codec_ctx{nullptr};
	int video_stream_index{-1};
	AVFrame* av_frame{nullptr};
	AVPacket* av_packet{nullptr};
	SwsContext* sws_scaler_ctx{nullptr};
};

class video_manager {

public:
	video_manager() = default;

	bool open_video(video_reader* state, const char* file);

	uint64_t get_video_length(const char* file);

	bool read_video_frame(GLFWwindow* window, video_reader* state, uint8_t* frame_buffer, int64_t* pts);

	void add_video(const video& video_to_add);

	void add_video(const std::string_view id, const std::string_view name, const float length);

	void remove_video(const std::string_view id);

	void add_option(video& vid, const std::string_view id, const std::string_view name, const std::string_view video_id);

	void update_option(video& vid, option& opt);

	void remove_all_videos();

	void render_window();

	std::map<std::string, video>& get_videos();

	video current_video;

	video next_video;

private:

	std::map<std::string, video> videos{};

};

