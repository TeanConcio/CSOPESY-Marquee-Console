#include "MarqueeConsole.h"



MarqueeConsole::MarqueeConsole(String name) {
	this->name = name;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Initialize hConsole
	GetConsoleScreenBufferInfo(hConsole, &csbi); // Initialize csbi
}

MarqueeConsole::~MarqueeConsole() {
}

// Enable the marquee console
void MarqueeConsole::onEnable() {
	this->running = true;

    // Disable the cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // Set the cursor visibility to false
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // Start the cursor blinking thread
    cursorThread = std::thread(&MarqueeConsole::toggleCursorVisibility, this);
}

// Disable the marquee console
void MarqueeConsole::onDisable() {
	this->running = false;

	// Enable the cursor
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = true; // Set the cursor visibility to true
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	// Stop the cursor blinking thread
	if (cursorThread.joinable()) {
		cursorThread.join();
	}
}

// Keyboard Polling
void MarqueeConsole::pollKeyboard(bool threading) {

    do {
        if (_kbhit()) { // Check if a key has been pressed
            // Get the pressed key
            char key = _getch();

            // Check if the key is an arrow key
            if (key == 0 || key == -32) {
                key = _getch();
                switch (key) {
                case 75: // Left arrow key
                    if (currentCursorPosition > 0) {
                        currentCursorPosition--;
                    }
                    break;
                case 77: // Right arrow key
                    if (currentCursorPosition < currentCommand.size()) {
                        currentCursorPosition++;
                    }
                    break;
                }
            }
            else if (key == 27) { // Escape key
                running = false;
                break;
            }
            else if (key == 13) { // Enter key
                currentCommand.push_back('\0');
                commandHistory.push_back(String(currentCommand.data()));
                currentCommand.clear();
                currentCursorPosition = 0;
            }
            else if (key == 8) { // Backspace key
                if (!currentCommand.empty() && currentCursorPosition > 0) {
                    currentCommand.erase(currentCommand.begin() + currentCursorPosition - 1);
                    currentCursorPosition--;
                }
            }
            else { // Other keys
                currentCommand.insert(currentCommand.begin() + currentCursorPosition, key);
                currentCursorPosition++;
            }
        }

		// If threading is enabled, sleep for a short delay
        if (threading)
            std::this_thread::sleep_for(
                std::chrono::milliseconds(MarqueeConsole::POLLING_DELAY)
            );
    } while (running && threading);
}

void MarqueeConsole::process() {
    // Update the marquee position
    marqueeX += marqueeXSpd;
    marqueeY += marqueeYSpd;

    // Reverse the speed if the marquee reaches the edge
    if (marqueeX <= 0 ||
        marqueeX >= marqueeWidth - marqueeTextSize - 1) {
        marqueeXSpd *= -1;
    }

    if (marqueeY <= 0 ||
        marqueeY >= marqueeHeight - 1) {
        marqueeYSpd *= -1;
    }
}

void MarqueeConsole::display() {
    // Get the current cursor position and screen buffer size
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    COORD cursorPos = csbi.dwCursorPosition;
    SHORT screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    bool screenFits = false;

    // Check if the outputs fit on the screen
    if (this->marqueeHeight + this->commandHistory.size() + 5 < screenHeight
        && false
    ) {
        // Move the cursor to the beginning of the console
        cursorPos.X = 0;
        cursorPos.Y = 0;
        SetConsoleCursorPosition(hConsole, cursorPos);

		screenFits = true;
    }
    else {
        // Clear the console
        system("cls");

		screenFits = false;
    }

    // Print the new output
    printHeader();
    printMarquee();

    std::cout << std::endl;

    printCurrentCommand();
    printCommandHistory();

	// Wait for a short delay before refreshing the display
    std::this_thread::sleep_for(std::chrono::milliseconds(MarqueeConsole::REFRESH_DELAY));

	// If screen fits, add more delay because the console is not cleared
	if (screenFits)
		std::this_thread::sleep_for(std::chrono::milliseconds(
            (MarqueeConsole::REFRESH_DELAY + 1) * 25 / (MarqueeConsole::REFRESH_DELAY + 1)
        ));
}

void MarqueeConsole::printHeader() {
    std::cout << "*****************************************" << std::endl;
    std::cout << "* Displaying a marquee console! *" << std::endl;
    std::cout << "*****************************************" << std::endl;
}

void MarqueeConsole::printMarquee() {
    for (int i = 0; i < marqueeHeight; i++) {
        if (i == marqueeY) {
            std::cout << 
                String(marqueeX, ' ') << 
                marqueeText << 
                String((marqueeWidth - (marqueeX + marqueeTextSize)), ' ') << 
                std::endl;
        }
        else {
            std::cout << String(marqueeWidth, ' ') << std::endl;
        }
    }
}

void MarqueeConsole::printCurrentCommand() {
    // Move the cursor to the beginning of the current command line
    GetConsoleScreenBufferInfo(hConsole, &csbi); // Reinitialize csbi
    COORD cursorPos = csbi.dwCursorPosition;
    cursorPos.X = 0;
    SetConsoleCursorPosition(hConsole, cursorPos);

    // Clear the current command line
    std::cout << String(csbi.dwSize.X, ' ');

    // Move the cursor back to the beginning of the current command line
    SetConsoleCursorPosition(hConsole, cursorPos);

    // Print the command prompt and the current command
    std::cout << commandPrompt;

    for (int i = 0; i < currentCommand.size(); ++i) {
        if (i == currentCursorPosition) {
            if (this->cursorVisible)
                std::cout << '_'; // Cursor indicator
            else
				std::cout << ' '; // Space to maintain the cursor position
        }
        std::cout << currentCommand[i];
    }

    // If the cursor is at the end of the command, show the cursor indicator
    if (currentCursorPosition == currentCommand.size()) {
        if (this->cursorVisible)
            std::cout << '_'; // Cursor indicator
        else
            std::cout << ' '; // Space to maintain the cursor position
    }

    std::cout << std::endl;
}

void MarqueeConsole::toggleCursorVisibility() {
    while (running) {
        cursorVisible = !cursorVisible;
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Toggle every 500ms
    }
}

void MarqueeConsole::printCommandHistory() {
    for (const String& command : commandHistory) {
        std::cout << "Command processed in MARQUEE_CONSOLE: " << command;

		// If not the last command, add a newline
		if (&command != &commandHistory.back())
			std::cout << std::endl;
    }
}
