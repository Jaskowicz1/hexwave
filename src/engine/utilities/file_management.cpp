#include <fstream>
#include <sstream>
#include <vector>
#include "utilities/file_management.h"

void utilities::save_project(video_manager& manager) {
	char filename[1024];
	FILE *f = popen("zenity --file-selection --save --title=\"Save project\"", "r");
	fgets(filename, 1024, f);

	json j;
	json video_array = json::array();

	for(const auto& vid_pair : manager.get_videos()) {
		video_array.push_back(vid_pair.second.to_json());
	}

	j["videos"] = video_array;

	std::ofstream project_file(filename);
	project_file << j.dump() << "\n";
	project_file.close();
}

void utilities::load_project(video_manager& manager) {
	char filename[1024];
	FILE *f = popen(R"(zenity --file-selection --title="Open project")", "r");
	fgets(filename, 1024, f);

	manager.remove_all_videos();

	std::ifstream project_file(filename);

	json j = json::parse(project_file);

	for (const auto& vid : j["videos"]) {
		manager.add_video(vid["id"].get<std::string>(), vid["name"].get<std::string>(), vid["length"].get<float>());
	}

	project_file.close();
}