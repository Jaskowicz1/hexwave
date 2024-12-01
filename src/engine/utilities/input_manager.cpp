#include <iostream>
#include <fstream>
#include "utilities/input_manager.h"

input::input_manager::input_manager(GLFWwindow* window) {
	if (!window) {
		return;
	}

	std::cout << "Setting up input manager..." << "\n";

	static input_manager *input_mgr = this;

	// Fire callback when keyboard input fires.
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		// Update input type to Keyboard/Mouse.
		input_mgr->current_input_type = KEYBOARDANDMOUSE;
		if (action == GLFW_PRESS) {
			input_mgr->keys_held.emplace_back(key);
			input_mgr->on_keyboard_press(key);
		}

		if(action == GLFW_RELEASE) {
			input_mgr->keys_held.erase(std::remove(input_mgr->keys_held.begin(),
							       input_mgr->keys_held.end(), key),
						   input_mgr->keys_held.end());
		}
	});

	// Fire callback when mouse moves.
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		// Update input type to Keyboard/Mouse.
		input_mgr->current_input_type = KEYBOARDANDMOUSE;
	});

	std::ifstream game_controller_db("extras/gamecontrollerdb.txt");

	if (!game_controller_db.good()) {
		std::cerr << "Could not find the game controller db file." << std::endl;
		abort();
	}

	std::string contents((std::istreambuf_iterator<char>(game_controller_db)),
			     std::istreambuf_iterator<char>());

	if (glfwUpdateGamepadMappings(contents.c_str()) == GLFW_FALSE) {
		std::cout << "Failed to update gamepad mappings." << "\n";
		return;
	}

	last_input = std::chrono::high_resolution_clock::now();

	std::cout << "Input manager has setup successfully!" << "\n";
}

input::input_manager::~input_manager() {
	std::cout << "Input manager killed." << "\n";
}

void input::input_manager::input_loop() {
	if (glfwJoystickPresent(GLFW_JOYSTICK_1) != GLFW_TRUE) {
		return;
	}

	const char *name = glfwGetJoystickName(GLFW_JOYSTICK_1);

	// Is the controller a Playstation Controller?
	if (std::string(name).find("DualSense") == 0) {
		current_controller_type = PLAYSTATION;
	} else {
		// Default to XBOX if we can't find "DualSense".
		current_controller_type = XBOX;
	}

	{
		int count;

		// Luckily, controller axis input on Playstation isn't any different to Xbox.
		const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

		const float x_axis = axes[0];
		const float y_axis = axes[1];

		if (abs(x_axis) >= dead_zone) {
			if (on_controller_input) {
				if (x_axis < 0) {
					on_controller_input(INPUT_LEFT);
				} else {
					on_controller_input(INPUT_RIGHT);
				}
			}

			current_input_type = CONTROLLER;
		}

		if (abs(y_axis) >= dead_zone) {
			if (on_controller_input) {
				if (y_axis < 0) {
					on_controller_input(INPUT_UP);
				} else {
					on_controller_input(INPUT_DOWN);
				}
			}

			current_input_type = CONTROLLER;
		}
	}

	GLFWgamepadstate state{};
	if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state) == GLFW_FALSE) {
		return;
	}

	const unsigned char* buttons = state.buttons;

	int button_pressed_index{ -1 };

	// state.buttons declares itself as "buttons[15]", so this is safe.
	for (uint8_t i = 0; i < 15; i++) {
		if (buttons[i] == GLFW_PRESS) {
			button_pressed_index = i;
			break;
		}
	}

	if (button_pressed_index != -1) {
		const auto now_time = std::chrono::high_resolution_clock::now();
		const auto result = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - last_input);
		if (result.count() >= time_required_between_input) {
			// Should always be true because we literally depend on this,
			// but never hurts to be safe.
			if (on_controller_button_press) {
				on_controller_button_press(static_cast<controller_inputs>(button_pressed_index));
			}

			current_input_type = CONTROLLER;
			last_input = std::chrono::high_resolution_clock::now();
		}
	}
}
