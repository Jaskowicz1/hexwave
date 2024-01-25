#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <map>

// ffmpeg moment
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

struct video {
	std::string id{};
	std::string name{};
	float length;
};

struct current_video_state {
	video vid;
	int width{};
	int height{};
	AVRational time_base;

	// Private internal state
	AVFormatContext* av_format_ctx;
	AVCodecContext* av_codec_ctx;
	int video_stream_index;
	AVFrame* av_frame;
	AVPacket* av_packet;
	//SwsContext* sws_scaler_ctx;
};

class video_manager {

public:
	video_manager() = default;

	void add_video(const video& video_to_add);

	void add_video(const std::string_view id, const std::string_view name, const float length);

	void remove_video(const std::string_view id);

	void remove_all_videos();

	void render_window();

	current_video_state current_video;

private:

	std::map<std::string, video> videos{};

};

