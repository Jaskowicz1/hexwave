#include <fstream>
#include <sstream>
#include <vector>
#include "utilities/file_management.h"

#ifdef _WIN32
#include <windows.h>
#endif

bool utilities::save_project(video_manager& manager) {

	char filename[1024];
	bool fail = false;

#ifndef _WIN32 // !_WIN32

	FILE *f = popen("zenity --file-selection --save --title=\"Save project\"", "r");
	fail = fgets(filename, 1024, f) == NULL;
	pclose(f);
	f = NULL;

#else // _WIN32

	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
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
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	GetSaveFileName(&ofn);

#endif

	if (fail || std::strlen(filename) == 0) {
		perror("fgets");
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
	bool fail = false;

#ifndef _WIN32 // !_WIN32

	FILE *f = popen(R"(zenity --file-selection --title="Open project")", "r");
	fail = fgets(filename, 1024, f) == NULL;
	pclose(f);
	f = NULL;

#else // _WIN32

	OPENFILENAME ofn;

	// open a file name
	ZeroMemory(&ofn, sizeof(ofn));
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

	if (fail || std::strlen(filename) == 0) {
		perror("fgets");
		return false;
	}

	manager.remove_all_videos();

	std::ifstream project_file(filename);

	if (project_file.bad()) {
		std::cout << "Failed to open file." << "\n";
		return false;
	}

	json j = json::parse(project_file);

	for (const auto& vid : j["videos"]) {
		manager.add_video(vid);

		for (const auto& opt : vid["options"]) {
			manager.add_option(manager.get_videos()[vid["id"].get<std::string>()],
					opt["id"].get<std::string>(),
					opt["name"].get<std::string>(), opt["video_id"].get<std::string>());
		}
	}

	project_file.close();

	return true;
}

// vim sw=4 ts=4 noet
