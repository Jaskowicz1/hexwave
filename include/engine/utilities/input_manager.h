#pragma once

#include <functional>
#include <chrono>
#include "GLFW/glfw3.h"

namespace input {

/**
 * @brief This is mapped to GLFW's GLFWgamepadstate buttons.
 *
 * @note This should match every controller. If not, check the gamecontrollerdb.txt in extras
 */
enum controller_inputs {
	/* Button Inputs */

	CONTROLLER_BUTTON_DOWN = 0,
	CONTROLLER_BUTTON_RIGHT = 1,
	CONTROLLER_BUTTON_LEFT = 2,
	CONTROLLER_BUTTON_UP = 3,
	CONTROLLER_BUTTON_LEFT_BUMPER = 4,
	CONTROLLER_BUTTON_RIGHT_BUMPER = 5,
	CONTROLLER_BUTTON_OPTION = 6,
	CONTROLLER_BUTTON_START = 7,
	CONTROLLER_BUTTON_SPECIAL = 8, // Xbox button, PS button, etc.
	CONTROLLER_BUTTON_LEFT_JOYSTICK = 9,
	CONTROLLER_BUTTON_RIGHT_JOYSTICK = 10,
	CONTROLLER_BUTTON_DPAD_UP = 11,
	CONTROLLER_BUTTON_DPAD_RIGHT = 12,
	CONTROLLER_BUTTON_DPAD_DOWN = 13,
	CONTROLLER_BUTTON_DPAD_LEFT = 14,

	/* Axis Inputs */

	INPUT_UP = 15,
	INPUT_DOWN = 16,
	INPUT_LEFT = 17,
	INPUT_RIGHT = 18,
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
	std::function<void(int key)> on_keyboard_press { nullptr };

	/**
	 * @brief Controller input event
	 *
	 * @param input The input of the controller as `controller_inputs`.
	 */
	std::function<void(controller_inputs input)> on_controller_input { nullptr };

	/**
	 * @brief Controller button press event
	 *
	 * @param key The controller button as `controller_inputs`.
	 */
	std::function<void(controller_inputs key)> on_controller_button_press { nullptr };

	std::vector<int> keys_held{};

	input_types current_input_type { input::KEYBOARDANDMOUSE };
	controller_types current_controller_type { input::XBOX };

private:

	/**
	 * @brief Time of last input.
	 *
	 * @note This is to prevent spam of a button.
	 * @see time_required_between_input
	 */
	std::chrono::time_point<std::chrono::high_resolution_clock> last_input;

	/**
	 * @brief Time (IN MS) required between last and current input.
	 */
	uint16_t time_required_between_input { 100 };

	/**
	 * @brief a Dead-Zone for the axis inputs.
	 *
	 * @note This prevents the smallest of movements from triggering input.
	 */
	float dead_zone { 0.2 };
};

}
