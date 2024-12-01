#include <fstream>
#include "project_settings.h"
#include "utilities/file_management.h"
#include "imgui.h"
#include "ImGuiNotify.hpp"

void project_settings::render_window() {
	if (!show_window) {
		return;
	}

	if (ImGui::Begin("Project Settings")) {
		static char start_vid_id[1024];
		if (!start_id.empty()) {
			std::memcpy(start_vid_id, start_id.c_str(), start_id.length());
		}

		if(ImGui::InputText("Starting Video ID", start_vid_id, IM_ARRAYSIZE(start_vid_id))) {
			start_id = start_vid_id;
		}

		static char button_texture_path[1024];
		if (!normal_button_path.empty()) {
			std::memcpy(button_texture_path, normal_button_path.c_str(), normal_button_path.length());
		}

		if(ImGui::InputText("Normal Button Texture Path", button_texture_path, IM_ARRAYSIZE(button_texture_path))) {
			normal_button_path = button_texture_path;
		}

		ImGui::SameLine();

		if (ImGui::Button(std::string("Select Image##001").c_str())) {
			std::string image_path{utilities::get_file_from_prompt(false, "Select Image", "Image File | *.png *.jpg", "PNG\0*.png\0JPEG\0*.jpg\0All\0*.*\0")};

			std::ifstream image_file(image_path);

			if (image_file.good()) {
				normal_button_path = image_path;
			} else {
				ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Failed to open Image file, please make sure it's a valid image file!" });
			}
		}

		static char hovered_button_texture_path[1024];
		if (!hovered_button_path.empty()) {
			std::memcpy(hovered_button_texture_path, hovered_button_path.c_str(), hovered_button_path.length());
		}

		if (ImGui::InputText("Hovered Button Texture Path", hovered_button_texture_path, IM_ARRAYSIZE(hovered_button_texture_path))) {
			hovered_button_path = hovered_button_texture_path;
		}

		ImGui::SameLine();

		if (ImGui::Button(std::string("Select Image##002").c_str())) {
			std::string image_path{utilities::get_file_from_prompt(false, "Select Image", "Image File | *.png *.jpg", "PNG\0*.png\0JPEG\0*.jpg\0All\0*.*\0")};

			std::ifstream image_file(image_path);

			if (image_file.good()) {
				hovered_button_path = image_path;
			} else {
				ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Failed to open Image file, please make sure it's a valid image file!" });
			}
		}

		static char selected_button_texture_path[1024];
		if (!selected_button_path.empty()) {
			std::memcpy(selected_button_texture_path, selected_button_path.c_str(), selected_button_path.length());
		}

		if (ImGui::InputText("Selected Button Texture Path", selected_button_texture_path, IM_ARRAYSIZE(selected_button_texture_path))) {
			selected_button_path = selected_button_texture_path;
		}

		ImGui::SameLine();

		if (ImGui::Button(std::string("Select Image##003").c_str())) {
			std::string image_path{utilities::get_file_from_prompt(false, "Select Image", "Image File | *.png *.jpg", "PNG\0*.png\0JPEG\0*.jpg\0All\0*.*\0")};

			std::ifstream image_file(image_path);

			if (image_file.good()) {
				selected_button_path = image_path;
			} else {
				ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Failed to open Image file, please make sure it's a valid image file!" });
			}
		}

		static char button_pressed_sound[1024];
		if (!button_sound_path.empty()) {
			std::memcpy(button_pressed_sound, button_sound_path.c_str(), button_sound_path.length());
		}

		if (ImGui::InputText("Button Pressed Sound Path", button_pressed_sound, IM_ARRAYSIZE(button_pressed_sound))) {
			button_sound_path = button_pressed_sound;
		}

		ImGui::SameLine();

		if (ImGui::Button(std::string("Select Sound").c_str())) {
			std::string sound_path{utilities::get_file_from_prompt(false, "Select Sound", "Sound File | *.mp3 *.wav", "MP3\0*.mp3\0WAV\0*.wav\0All\0*.*\0")};

			std::ifstream sound_file(sound_path);

			if (sound_file.good()) {
				button_sound_path = sound_path;
			} else {
				ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Failed to open Sound file, please make sure it's a valid sound file!" });
			}
		}
	}
	ImGui::End();
}

project_settings &project_settings::fill_from_json(const json *j) {
	utilities::set_string_not_null(j, "start_id", start_id);
	utilities::set_string_not_null(j, "normal_button_path", normal_button_path);
	utilities::set_string_not_null(j, "hovered_button_path", hovered_button_path);
	utilities::set_string_not_null(j, "selected_button_path", selected_button_path);
	utilities::set_string_not_null(j, "button_sound_path", button_sound_path);

	return *this;
}

json project_settings::to_json() const {
	json j;

	j["start_id"] = start_id;
	j["normal_button_path"] = normal_button_path;
	j["hovered_button_path"] = hovered_button_path;
	j["selected_button_path"] = selected_button_path;
	j["button_sound_path"] = button_sound_path;

	return j;
}
