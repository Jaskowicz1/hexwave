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
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavutil/audio_fifo.h>
}

struct video_reader {
	int width{0};
	int height{0};

	int video_stream_index{-1};
	int audio_stream_index{-1};

	AVRational time_base;
	AVFormatContext* av_format_ctx{nullptr};

	AVCodecContext* av_codec_ctx_video{nullptr};
	AVCodecContext* av_codec_ctx_audio{nullptr};

	AVFrame* av_frame{nullptr};
	AVPacket* av_packet{nullptr};
	SwsContext* sws_scaler_ctx{nullptr};
	SwrContext* swr_resampler_ctx{nullptr};

	AVAudioFifo* av_audio_fifo{nullptr};
	AVPacket* av_audio_packet{nullptr};
	AVFrame* av_audio_frame{nullptr};
};

class video_manager {

public:
	video_manager() = default;

	bool open_video(video_reader* state, const video& vid);

	uint64_t get_video_length(const char* file);

	bool read_video_frame(GLFWwindow* window, video_reader* state, uint8_t* frame_buffer, int64_t* pts);

	void add_video(const video& video_to_add);

	void add_video(const std::string_view id, const std::string_view name, const uint64_t length, const std::string_view path);

	void remove_video(const std::string_view id);

	void add_option(video& vid, const std::string_view id, const std::string_view name, const std::string_view video_id);

	void update_option(video& vid, option& opt);

	void remove_all_videos();

	void render_window(video_reader& reader);

	std::map<std::string, video>& get_videos();

	video current_video;

	video next_video;

private:

	std::map<std::string, video> videos{};

};

