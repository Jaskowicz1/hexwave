#include <iostream>
#include "window.h"

static bool testing_export = false;

// Main code
int main(int argc, char* argv[]) {
	// Create new window instance.
	window win{};

	// Do the window loop.
	win.window_loop();

	// window de-constructor will call as window_loop has finished;
	return 0;
}
