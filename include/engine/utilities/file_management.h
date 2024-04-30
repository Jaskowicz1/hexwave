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
struct linux_file {
	FILE* f;

	operator FILE *() {return f;}

	~linux_file() {
		pclose(f);
		f = nullptr;
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
std::string get_file_from_prompt(const bool is_save, const std::string_view title, const std::vector<std::string_view>& filters);
}