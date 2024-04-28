#include <fstream>
#include <sstream>
#include <vector>
#include "utilities/file_management.h"

#ifdef _WIN32
#include <windows.h>
#endif

bool utilities::save_project(video_manager& manager) {

	char filename[1024];

	// Linux only, will ALWAYS be false on Windows.
	bool file_fail{ false };

#ifndef _WIN32 // !_WIN32

	FILE* f = popen(R"(zenity --file-selection --save --title="Save project" --file-filter=*.hexw)", "r");

	// Might be possible to just not use fgets, should look into this.
	// Will be true if we failed to gather the data from the file.
	file_fail = (fgets(filename, 1024, f) == nullptr);

	pclose(f);
	f = nullptr;

#else // _WIN32

	OPENFILENAME ofn{};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "Hexwave Project\0*.hexw\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	GetSaveFileName(&ofn);

#endif

	if (file_fail || std::strlen(filename) == 0) {
		return false;
	}

	json j;
	json video_array = json::array();

	for (const auto& vid_pair : manager.get_videos()) {
		video_array.push_back(vid_pair.second.to_json());
	}

	j["videos"] = video_array;

	std::ofstream project_file(filename);
	project_file << j.dump() << "\n";
	project_file.close();

	return true;
}

bool utilities::load_project(video_manager& manager) {

	char filename[1024];

	// Linux only, will ALWAYS be false on Windows.
	bool file_fail{ false };

#ifndef _WIN32 // !_WIN32

	FILE* f = popen(R"(zenity --file-selection --title="Open project" --file-filter=*.hexw)", "r");

	// Might be possible to just not use fgets, should look into this.
	// Will be true if fgets is nullptr (invalid).
	file_fail = (fgets(filename, 1024, f) == nullptr);

	pclose(f);
	f = nullptr;

#else // _WIN32

	OPENFILENAME ofn{};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = "Hexwave Project\0*.hexw\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	GetOpenFileName(&ofn);

#endif

	if (file_fail || std::strlen(filename) == 0) {
		return false;
	}

	std::ifstream project_file(filename);

	if (project_file.bad()) {
		/* 
		 * We should ideally change the return value of file_management with an enum of the result,
		 * meaning we can accurately say the reason for a failure.
		 */
		std::cout << "Failed to open file." << "\n";
		return false;
	}

	try {
		json j = json::parse(project_file);

		// Clear videos if parse succeeded.
		manager.remove_all_videos();

		for (const auto& vid : j["videos"]) {
			manager.add_video(vid);

			for (const auto& opt : vid["options"]) {
				manager.add_option(manager.get_videos()[vid["id"].get<std::string>()], opt["id"].get<std::string>(),
					opt["name"].get<std::string>(), opt["video_id"].get<std::string>());
			}
		}
	}
	catch (json::parse_error& exception) {
		project_file.close();
		return false;
	}

	project_file.close();

	return true;
}
