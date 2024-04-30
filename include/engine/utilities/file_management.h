#pragma once

#include "video_manager.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace utilities {

#ifndef WIN32
/**
 * @brief Linux File Reader.
 *
 * @note linux_file will automatically close and free up memory when it is no longer required.
 */
class linux_file {
	FILE* f{nullptr};

public:
	linux_file() = default;
	linux_file(FILE *file) : f{file} {}
	linux_file(const linux_file&) = delete;
	linux_file(linux_file&& other) noexcept : f{std::exchange(other.f, nullptr)} {}

	linux_file &operator=(const linux_file&) = delete;
	linux_file &operator=(linux_file&& other) noexcept {
		f = std::exchange(other.f, nullptr);
		return *this;
	}

	operator FILE *() {return f;}

	~linux_file() {
		if(f != nullptr) {
			pclose(f);
			f = nullptr;
		}
	};
};
#endif

bool save_project(video_manager& manager);

bool load_project(video_manager& manager);

/**
 * @brief Opens a file prompt to retrieve a file path.
 *
 *
 * @return
 */
std::string get_file_from_prompt(const bool is_save, const std::string_view title, const std::vector<std::string_view>& filters = {});
}