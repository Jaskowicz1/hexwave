#pragma once

#include <functional>
#include <chrono>
#include "GLFW/glfw3.h"

namespace input {

enum controller_inputs {
	INPUT_UP = 0,
	INPUT_DOWN = 1,
	INPUT_LEFT = 2,
	INPUT_RIGHT = 3,
	CONTROLLER_BUTTON_UP = 4,
	CONTROLLER_BUTTON_DOWN = 5,
	CONTROLLER_BUTTON_LEFT = 6,
	CONTROLLER_BUTTON_RIGHT = 7,
	CONTROLLER_BUTTON_OPTION = 8,
	CONTROLLER_BUTTON_START = 9,
	CONTROLLER_BUTTON_LEFT_JOYSTICK = 10,
	CONTROLLER_BUTTON_RIGHT_JOYSTICK = 11,
	CONTROLLER_BUTTON_LEFT_BUMPER = 12,
	CONTROLLER_BUTTON_LB = 12,
	CONTROLLER_BUTTON_RIGHT_BUMPER = 13,
	CONTROLLER_BUTTON_RB = 13,
	CONTROLLER_BUTTON_DPAD_UP = 14,
	CONTROLLER_BUTTON_DPAD_DOWN = 15,
	CONTROLLER_BUTTON_DPAD_LEFT = 16,
	CONTROLLER_BUTTON_DPAD_RIGHT = 17,
	CONTROLLER_BUTTON_SPECIAL = 18, // Xbox button, PS button, etc.
};

enum input_types {
	KEYBOARDANDMOUSE,
	CONTROLLER,
};

enum controller_types {
	XBOX,
	PLAYSTATION
};

class input_manager {

public:
	/**
	 * @brief input_manager constructor.
	 * @param window The window reference to setup the input_manager.
	 */
	input_manager(GLFWwindow* window);

	~input_manager();

	void input_loop();

	/**
	 * @brief Keyboard press event
	 *
	 * @param key The key that was pressed (look at `GLFW_KEY_`).
	 *
	 */
	std::function<void(int key)> on_keyboard_press{nullptr};

	/**
	 * @brief Controller input event
	 *
	 * @param input The input of the controller as `controller_inputs`.
	 */
	std::function<void(controller_inputs input)> on_controller_input{nullptr};

	/**
	 * @brief Controller button press event
	 *
	 * @param key The controller button as `controller_inputs`.
	 */
	std::function<void(controller_inputs key)> on_controller_button_press{nullptr};

	std::vector<int> keys_held{};

	input_types current_input_type{input::KEYBOARDANDMOUSE};
	controller_types current_controller_type{input::XBOX};

private:

	std::chrono::time_point<std::chrono::high_resolution_clock> last_input;
	float deadzone = 0.2;
};

}
