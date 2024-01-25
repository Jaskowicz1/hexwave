#include <iostream>
#include "video_manager.h"
#include "imgui.h"

void video_manager::add_video(const video& video_to_add) {
	videos.emplace(video_to_add.id, video_to_add);
}

void video_manager::add_video(const std::string_view id, const std::string_view name, const float length) {
	video vid;
	vid.id = id;
	vid.name = name;
	vid.length = length;
	videos.emplace(id, vid);
}

void video_manager::remove_video(const std::string_view id) {
	if(videos.find(std::string(id.begin(), id.end())) != videos.end()) {
		videos.erase(std::string(id.begin(), id.end()));
	}
}

void video_manager::remove_all_videos() {
	videos.clear();
}

void video_manager::render_window() {
	if(ImGui::Begin("Videos")) {
		if (ImGui::Button("Add Video")) {
			ImGui::OpenPopup("add_video_popup");
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove ALL Videos")) {
			remove_all_videos();
		}

		if (ImGui::BeginPopup("add_video_popup")) {
			static char id[64];
			ImGui::InputText("ID", id, IM_ARRAYSIZE(id));

			static char name[64];
			ImGui::InputText("Name", name, IM_ARRAYSIZE(name));

			if (ImGui::Button("Add")) {
				if(strlen(id) != 0 && strlen(name) != 0) {
					add_video(id, name, 1.f);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		if(ImGui::BeginChild("Scrolling")) {
			if(!videos.empty()) {
				const auto temp_videos = videos;
				for (const auto& vid_pair : temp_videos) {
					const video& vid = vid_pair.second;
					std::string title("Video: " + vid.id);
					if (ImGui::CollapsingHeader(title.c_str())) {
						// --------------------------------------------------
						ImGui::SeparatorText("Video Information");

						std::string id_txt("Video name: " + vid.name);
						ImGui::Text("%s", id_txt.c_str());
						std::string length_txt("Video length: " + std::to_string(vid.length));
						ImGui::Text("%s", length_txt.c_str());

						// --------------------------------------------------
						ImGui::SeparatorText("Video Management");

						if (ImGui::Button("Play")) {
							std::cout << vid.name << "\n";
						}
						ImGui::SameLine();
						if (ImGui::Button("Move Up")) {
						}
						ImGui::SameLine();
						if (ImGui::Button("Move Down")) {
						}
						ImGui::SameLine();
						if (ImGui::Button("Remove")) {
							remove_video(vid.id);
						}

						// --------------------------------------------------
						ImGui::SeparatorText("End");
					}
				}
			} else {
				ImGui::Text("There are no videos!");
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}
}

std::map<std::string, video> &video_manager::get_videos() {
	return videos;
}
