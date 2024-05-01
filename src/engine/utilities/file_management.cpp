#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include "utilities/file_management.h"

bool utilities::save_project(video_manager& manager) {

	std::string filename{get_file_from_prompt(true, "Save Project", "Hexwave Project | *.hexw", "Hexwave Project\0*.hexw\0")};

	if(filename.empty()) {
		return false;
	}

	// Does filename NOT end in ".hexw"?
	if(filename.compare(filename.size() - hexwave_project_ext.size(), hexwave_project_ext.size(), hexwave_project_ext) != 0) {
		filename.append(hexwave_project_ext);
	}

	json j;
	json video_array = json::array();

	for (const auto& vid_pair : manager.get_videos()) {
		video_array.push_back(vid_pair.second.to_json());
	}

	j["videos"] = video_array;

	// will auto close on deconstructor
	std::ofstream project_file(filename);
	project_file << j.dump() << "\n";

	return true;
}

bool utilities::load_project(video_manager& manager) {

	std::string filename{get_file_from_prompt(false, "Open Project", "Hexwave Project | *.hexw", "Hexwave Project\0*.hexw\0")};

	if(filename.empty()) {
		return false;
	}

	std::ifstream project_file(filename);

	if (!project_file.good()) {
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
	catch (const json::parse_error& exception) {
		std::cout << exception.what() << "\n";
		return false;
	}

	return true;
}

std::string utilities::get_file_from_prompt(const bool is_save, const std::string& title,
					    const std::string& linux_filters, const char* windows_filters) {
	char filename[1024];

#ifndef _WIN32 // !_WIN32

	std::string clause("zenity --file-selection" + std::string(is_save ? " --save" : "") + " --title=" + title + " --file-filter='" + linux_filters + "' --file-filter='All | *.*'");

	linux_file f{popen(clause.c_str(), "r")};

	// Might be possible to just not use fgets, should look into this.
	// Will be true if fgets is nullptr (invalid).
	if(fgets(filename, 1024, f) == nullptr) {
		return "";
	}

#else // _WIN32

	OPENFILENAME ofn{};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = windows_filters;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.lpstrTitle = std::string(title).c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(is_save) {
		GetSaveFileName(&ofn);
	} else {
		GetOpenFileName(&ofn);
	}

#endif

	if(std::strlen(filename) == 0) {
		return "";
	}

	auto len = std::strlen(filename);
	if (len > 0 && filename[len - 1] == '\n') {
		filename[len - 1] = 0;
	}

	return filename;
}
