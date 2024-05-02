#pragma once

#include "video_manager.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace utilities {

constexpr std::string_view hexwave_project_ext = ".hexw";

#ifndef _WIN32
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

/**
 * @brief Save the current project
 *
 * @param manager The video manager with the videos.
 *
 * @return bool true if project was able to save
 */
bool save_project(video_manager& manager);

/**
 * @brief Load a project.
 *
 * @brief The video manager to load the videos to.
 *
 * @return bool true if project was loaded successfully.
 */
bool load_project(video_manager& manager);

/**
 * @brief Opens a file prompt to retrieve a file path.
 *
 * @param is_save Is this prompt a save prompt?
 * @param title The title of the prompt.
 * @param linux_filters Filters for Linux (usually formatted as `name | filter`)
 * @param windows_filters Filters for Windows (usually formatted as `name\0filter\0name\0filter\0`)
 *
 * @return std::string File path (includes file name, for example `/opt/hexwave/testing.hexw`
 */
std::string get_file_from_prompt(const bool is_save, const std::string& title, const std::string& linux_filters, const char* windows_filters);
}