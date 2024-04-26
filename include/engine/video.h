#pragma once

#include "utilities/json_utilities.h"

struct option {
	std::string id{};
	std::string name{};
	std::string video_id{};
};

struct video {
	std::string id{};
	std::string name{};
	double length{0};
	std::string path{};
	std::string next_video_id{};
	bool loop{false};

	bool always_show_options{false};
	uint64_t options_show_at{0};
	uint64_t options_hide_at{0}; // if left at 0 (or less than show_at), this will be ignored.

	std::map<std::string, option> options{};

	video& fill_from_json(const json* j);

	json to_json() const;
};