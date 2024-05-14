#include <iostream>
#include <fstream>
#include "utilities/input_manager.h"

input::input_manager::input_manager(GLFWwindow* window) {
	if(!window) {
		return;
	}

	std::cout << "Setting up input manager..." << "\n";

	static input_manager *mgr = this;

	// Fire callback when keyboard input fires.
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		// Update input type to Keyboard/Mouse.
		mgr->current_input_type = KEYBOARDANDMOUSE;
		if(action == GLFW_PRESS) {
			mgr->keys_held.emplace_back(key);
			mgr->on_keyboard_press(key);
		}
	});

	// Fire callback when mouse moves.
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		// Update input type to Keyboard/Mouse.
		mgr->current_input_type = KEYBOARDANDMOUSE;
	});

	std::ifstream game_controller_db("extras/gamecontrollerdb.txt");

	if (!game_controller_db.good()) {
		std::cerr << "Could not find the game controller db file." << std::endl;
		abort();
	}

	std::string contents((std::istreambuf_iterator<char>(game_controller_db)),
			     std::istreambuf_iterator<char>());

	if(glfwUpdateGamepadMappings(contents.c_str()) == GLFW_FALSE) {
		std::cout << "Failed to update gamepad mappings." << "\n";
		return;
	}

	std::cout << "Input manager has setup successfully!" << "\n";
}

input::input_manager::~input_manager() {
	std::cout << "Input manager killed." << "\n";
}

void input::input_manager::input_loop() {
	if(glfwJoystickPresent(GLFW_JOYSTICK_1) == GLFW_TRUE) {

		const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);

		// Is the controller a Playstation Controller?
		if(std::string(name).find("DualSense") == 0) {
			current_controller_type = PLAYSTATION;
		} else {
			// Default to XBOX if we can't find "DualSense".
			current_controller_type = XBOX;
		}

		int count;

		// Luckily, controller axis input on Playstation isn't any different to Xbox.
		if(on_controller_input) {
			const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

			const float x_axis = axes[0];
			const float y_axis = axes[1];

			if(abs(x_axis) >= deadzone) {
				if(x_axis < 0) {
					on_controller_input(INPUT_LEFT);
				} else {
					on_controller_input(INPUT_RIGHT);
				}

				current_input_type = CONTROLLER;
			}

			if(abs(y_axis) >= deadzone) {
				if(y_axis < 0) {
					on_controller_input(INPUT_UP);
				} else {
					on_controller_input(INPUT_DOWN);
				}

				current_input_type = CONTROLLER;
			}
		}

		GLFWgamepadstate state{};
		if(glfwGetGamepadState(GLFW_JOYSTICK_1, &state) == GLFW_FALSE) {
			return;
		}

		const unsigned char* buttons = state.buttons;

		// Definitely a better way to do below code (mapping buttons to the actual index of the array?) but oh well.
		if(on_controller_button_press) {
			if(buttons[0] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_DOWN);
				current_input_type = CONTROLLER;
			}

			if(buttons[1] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_RIGHT);
				current_input_type = CONTROLLER;
			}

			if(buttons[2] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_LEFT);
				current_input_type = CONTROLLER;
			}

			if(buttons[3] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_UP);
				current_input_type = CONTROLLER;
			}

			if(buttons[4] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_LEFT_BUMPER);
				current_input_type = CONTROLLER;
			}

			if(buttons[5] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_RIGHT_BUMPER);
				current_input_type = CONTROLLER;
			}

			if(buttons[6] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_OPTION);
				current_input_type = CONTROLLER;
			}

			if(buttons[7] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_START);
				current_input_type = CONTROLLER;
			}

			if(buttons[8] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_SPECIAL);
				current_input_type = CONTROLLER;
			}

			if(buttons[9] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_LEFT_JOYSTICK);
				current_input_type = CONTROLLER;
			}

			if(buttons[10] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_RIGHT_JOYSTICK);
				current_input_type = CONTROLLER;
			}

			if(buttons[11] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_DPAD_UP);
				current_input_type = CONTROLLER;
			}

			if(buttons[12] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_DPAD_RIGHT);
				current_input_type = CONTROLLER;
			}

			if(buttons[13] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_DPAD_DOWN);
				current_input_type = CONTROLLER;
			}

			if(buttons[14] == GLFW_PRESS) {
				on_controller_button_press(CONTROLLER_BUTTON_DPAD_LEFT);
				current_input_type = CONTROLLER;
			}
		}
	}
}
