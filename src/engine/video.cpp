#include "video.h"

video& video::fill_from_json(const json *j) {
	utilities::set_string_not_null(j, "id", id);
	utilities::set_string_not_null(j, "name", name);
	utilities::set_uint64_not_null(j, "length", length);
	utilities::set_string_not_null(j, "path", path);
	utilities::set_string_not_null(j, "linked_video", next_video_id);
	utilities::set_bool_not_null(j, "always_show_options", always_show_options);
	utilities::set_uint64_not_null(j, "options_show_start_time", options_show_at);
	utilities::set_uint64_not_null(j, "options_show_end_time", options_hide_at);

	return *this;
}

json video::to_json() const {
	json j;

	j["id"] = id;
	j["name"] = name;
	j["length"] = length;
	j["path"] = path;
	j["linked_video"] = next_video_id;
	j["loop"] = next_video_id;
	j["always_show_options"] = always_show_options;
	j["options_show_start_time"] = options_show_at;
	j["options_show_end_time"] = options_hide_at;

	json options_array = json::array();

	for(const auto& opt_pair : options) {
		const option& opt = opt_pair.second;
		json opt_json;
		opt_json["id"] = opt.id;
		opt_json["name"] = opt.name;
		opt_json["video_id"] = opt.video_id;
		options_array.push_back(opt_json);
	}

	j["options"] = options_array;

	return j;

}
