#include "video.h"

video& video::fill_from_json(const json *j) {
	utilities::set_string_not_null(j, "id", id);
	utilities::set_string_not_null(j, "name", name);
	utilities::set_float_not_null(j, "length", length);

	return *this;
}

json video::to_json() const {
	json j;

	j["id"] = id;
	j["name"] = name;
	j["length"] = length;

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
