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
	//j["options"] = length;

	return j;

}
