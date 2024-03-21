#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

namespace utilities {

template <typename T>
T& fill_from_json(T&& obj, const json* j) {
	return std::forward<T&&>(obj).fill_from_json(j);
}

template <typename T>
auto to_json(T&& obj) {
	return std::forward<T&&>(obj).to_json();
}

template <typename T>
std::string string_json(T&& obj) {
	return to_json(obj).dump();
}

inline void set_string_not_null(const json* j, const char* keyname, std::string& value) {
	auto key = j->find(keyname);
	if(key != j->end()) {
		value = !key->is_null() && key->is_string() ? key->get<std::string>() : "";
	}
}

inline void set_float_not_null(const json* j, const char* keyname, float& value) {
	auto key = j->find(keyname);
	if(key != j->end()) {
		value = !key->is_null() && !key->is_string() ? key->get<float>() : 0.f;
	}
}

inline void set_uint64_not_null(const json* j, const char* keyname, uint64_t& value) {
	auto key = j->find(keyname);
	if(key != j->end()) {
		value = !key->is_null() && !key->is_string() ? key->get<uint64_t>() : 0;
	}
}

inline void set_bool_not_null(const json* j, const char* keyname, bool& value) {
	auto key = j->find(keyname);
	if(key != j->end()) {
		value = !key->is_null() && !key->is_string() && key->get<bool>();
	}
}

}