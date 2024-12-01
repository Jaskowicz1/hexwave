#pragma once

#include <string>
#include "utilities/json_utilities.h"

class project_settings {

public:
	project_settings() = default;

	void render_window();

	bool show_window;

	std::string start_id{};

	std::string normal_button_path{};
	std::string hovered_button_path{};
	std::string selected_button_path{};

	std::string button_sound_path{};

	project_settings& fill_from_json(const json* j);

	json to_json() const;

};