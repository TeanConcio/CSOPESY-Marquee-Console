#include "MarqueeConsole.h"


const bool THREADING = false;

int main() {
    MarqueeConsole marqueeConsole;

	if (THREADING) {
		// Start the keyboard polling in a separate thread using a lambda
		std::thread keyboardThread([&marqueeConsole]() {
			marqueeConsole.pollKeyboard(true);
			});

		while (marqueeConsole.running) { // Loop to keep updating the display
			marqueeConsole.process();
			marqueeConsole.display();
		}

		// Join the keyboard thread before exiting
		keyboardThread.join();
	}
	else {
		while (marqueeConsole.running) { // Loop to keep updating the display
			marqueeConsole.pollKeyboard(false);
			marqueeConsole.process();
			marqueeConsole.display();
		}
	}

    return 0;
}
