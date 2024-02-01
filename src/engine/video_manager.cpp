#include <iostream>
#include "video_manager.h"
#include "imgui.h"
#include "imgui_internal.h"

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

void video_manager::add_option(video& vid, const std::string_view id, const std::string_view name, const std::string_view video_id) {
	option opt;
	opt.id = id;
	opt.name = name;
	opt.video_id = video_id;
	vid.options.emplace(id, opt);
}

static void testing() {
	ImGui::OpenPopup("add_video_popup");
}


void video_manager::render_window() {
	if(ImGui::Begin("Videos")) {
		if (ImGui::Button("Add Video")) {
			testing();
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove ALL Videos")) {
			//remove_all_videos();
		}

		static std::string vid_id_interacting_with;

		if(ImGui::BeginChild("Scrolling")) {

			if (ImGui::BeginPopup("add_options_popup")) {
				static char id[64];
				ImGui::InputText("ID", id, IM_ARRAYSIZE(id));

				static char name[64];
				ImGui::InputText("Name", name, IM_ARRAYSIZE(name));

				static char video_id[64];
				ImGui::InputText("Option linked to video ID", video_id, IM_ARRAYSIZE(video_id));

				if (ImGui::Button("Add")) {
					if(strlen(id) != 0 && strlen(name) != 0 && strlen(video_id) != 0) {
						add_option(videos[vid_id_interacting_with], id, name, video_id);
						ImGui::CloseCurrentPopup();
						vid_id_interacting_with = "";
					}
				}

				ImGui::EndPopup();
			}

			if(!videos.empty()) {
				auto temp_videos = videos;
				for (auto& vid_pair : temp_videos) {
					video& vid = vid_pair.second;
					std::string title("Video: " + vid.id);
					if (ImGui::CollapsingHeader(title.c_str())) {
						// --------------------------------------------------
						ImGui::SeparatorText("Video Information");

						std::string id_txt("Video name: " + vid.name);
						ImGui::Text("%s", id_txt.c_str());
						std::string length_txt("Video length: " + std::to_string(vid.length));
						ImGui::Text("%s", length_txt.c_str());

						// --------------------------------------------------

						ImGui::SeparatorText("Options Information");

						if(!vid.options.empty()) {
							if (ImGui::CollapsingHeader("Options")) {
								for (auto& opt_pair : vid.options) {
									option& opt = opt_pair.second;
									std::string opt_title("Option: " + opt.id);
									if (ImGui::TreeNode(opt_title.c_str())) {
										// --------------------------------------------------
										ImGui::SeparatorText("Option Information");

										std::string opt_id("Option ID: " + opt.id);
										ImGui::Text("%s", opt_id.c_str());

										std::string opt_txt("Option name: " + opt.name);
										ImGui::Text("%s", opt_txt.c_str());

										std::string opt_video_id("Option linked to video ID: " + opt.video_id);
										ImGui::Text("%s", opt_video_id.c_str());

										// --------------------------------------------------
										ImGui::SeparatorText("End");

										ImGui::TreePop();
									}
								}
							}
						} else {
							ImGui::Text("There are no options!");
						}

						if (ImGui::Button(std::string("Add Option###" + vid.id).c_str())) {
							vid_id_interacting_with = vid.id;
							ImGui::OpenPopup("add_options_popup");
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Remove All###" + vid.id).c_str())) {
							// Do removal of all options here.
						}

						// --------------------------------------------------
						ImGui::SeparatorText("Video Management");

						if (ImGui::Button(std::string("Play###" + vid.id).c_str())) {
							std::cout << vid.name << "\n";
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Move Up###" + vid.id).c_str())) {
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Move Down###" + vid.id).c_str())) {
						}
						ImGui::SameLine();
						if (ImGui::Button(std::string("Remove Video###" + vid.id).c_str())) {
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

		ImGui::End();
	}
}

std::map<std::string, video> &video_manager::get_videos() {
	return videos;
}

void video_manager::update_option(video &vid, option &opt) {
	auto x = vid.options.find(opt.id);
	if(x != vid.options.end()) {
		x->second.name = opt.name;
		x->second.video_id = opt.video_id;
	}
}
