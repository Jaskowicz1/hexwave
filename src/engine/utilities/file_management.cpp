#include <fstream>
#include <sstream>
#include <vector>
#include "utilities/file_management.h"

bool utilities::save_project(video_manager& manager) {
#ifndef _WIN32

	char filename[1024];
	FILE *f = popen("zenity --file-selection --save --title=\"Save project\"", "r");
	fgets(filename, 1024, f);

	if (std::strlen(filename) == 0) {
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

#endif // !_WIN32
	return true;
}

bool utilities::load_project(video_manager& manager) {

#ifndef _WIN32

	char filename[1024];
	FILE *f = popen(R"(zenity --file-selection --title="Open project")", "r");
	fgets(filename, 1024, f);

	if(std::strlen(filename) == 0) {
		return false;
	}

	manager.remove_all_videos();

	std::ifstream project_file(filename);

	json j = json::parse(project_file);

	for (const auto& vid : j["videos"]) {
		manager.add_video(vid);

		for (const auto& opt : vid["options"]) {
			manager.add_option(manager.get_videos()[vid["id"].get<std::string>()], opt["id"].get<std::string>(),
			        opt["name"].get<std::string>(), opt["video_id"].get<std::string>());
		}
	}

	project_file.close();

#endif // !_WIN32

	return true;
}