#include <iostream>
#include "utilities/input_manager.h"

input::input_manager::input_manager(GLFWwindow* window) {
	if(!window) {
		return;
	}

	std::cout << "Setting up input manager..." << "\n";

	static input_manager *mgr = this;

	glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		mgr->current_input_type = input_types::KeyboardAndMouse;
		if(action == GLFW_PRESS) {
			mgr->keys_held.emplace_back(key);
			mgr->on_keyboard_press(key);
		}
	});

	std::cout << "Input manager has setup successfully!" << "\n";
}

input::input_manager::~input_manager() {
	std::cout << "Input manager killed." << "\n";
}

void input::input_manager::input_loop() {
	if(glfwJoystickPresent(GLFW_JOYSTICK_1) == GLFW_TRUE) {
		int count;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

		const float x_axis = axes[0];
		const float y_axis = axes[1];

		if(on_controller_input) {
			if(abs(x_axis) >= deadzone) {
				if(x_axis < 0) {
					on_controller_input(controller_inputs::INPUT_LEFT);
				} else {
					on_controller_input(controller_inputs::INPUT_RIGHT);
				}

				current_input_type = input_types::Controller;
			}

			if(abs(y_axis) >= deadzone) {
				if(y_axis < 0) {
					on_controller_input(controller_inputs::INPUT_UP);
				} else {
					on_controller_input(controller_inputs::INPUT_DOWN);
				}

				current_input_type = input_types::Controller;
			}
		}

		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);

		if(on_controller_button_press) {
			if(buttons[0] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_DOWN);
				current_input_type = input_types::Controller;
			}

			if(buttons[1] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_RIGHT);
				current_input_type = input_types::Controller;
			}

			if(buttons[2] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_LEFT);
				current_input_type = input_types::Controller;
			}

			if(buttons[3] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_UP);
				current_input_type = input_types::Controller;
			}

			if(buttons[4] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_LEFT_BUMPER);
				current_input_type = input_types::Controller;
			}

			if(buttons[5] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_RIGHT_BUMPER);
				current_input_type = input_types::Controller;
			}

			if(buttons[6] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_OPTION);
				current_input_type = input_types::Controller;
			}

			if(buttons[7] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_START);
				current_input_type = input_types::Controller;
			}

			if(buttons[8] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_LEFT_JOYSTICK);
				current_input_type = input_types::Controller;
			}

			if(buttons[9] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_RIGHT_JOYSTICK);
				current_input_type = input_types::Controller;
			}

			if(buttons[10] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_DPAD_UP);
				current_input_type = input_types::Controller;
			}

			if(buttons[11] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_DPAD_RIGHT);
				current_input_type = input_types::Controller;
			}

			if(buttons[12] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_DPAD_DOWN);
				current_input_type = input_types::Controller;
			}

			if(buttons[13] == GLFW_PRESS) {
				on_controller_button_press(controller_inputs::CONTROLLER_BUTTON_DPAD_LEFT);
				current_input_type = input_types::Controller;
			}
		}
	}
}
