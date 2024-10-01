#pragma once

#include <iostream>
#include <vector>
#include <windows.h>
#include <string>
#include <conio.h>
#include <thread>
#include <chrono>



class MarqueeConsole
{
public:
	MarqueeConsole();
	~MarqueeConsole();

	bool running = true;

	void pollKeyboard(bool threading);
	void process();
	void display();



private:
	static const int REFRESH_DELAY = 1;
	static const int POLLING_DELAY = 1;

	HANDLE hConsole; // Console handle
	CONSOLE_SCREEN_BUFFER_INFO csbi; // Console screen buffer info

	int marqueeWidth = 100;
	int marqueeHeight = 20;
	int marqueeXSpd = 1;
	int marqueeYSpd = 1;

	std::string marqueeText = "Hello World";
	int marqueeTextSize = marqueeText.length();
	int marqueeX = 0;
	int marqueeY = 0;

	std::string commandPrompt = "Enter a command for MARQUEE_CONSOLE: ";
	int commandPromptSize = commandPrompt.length();
	int currentCursorPosition = 0;
	std::vector<std::string> commandHistory;
	std::vector<char> currentCommand;

	bool cursorVisible = true;
	std::thread cursorThread;

	void printHeader();
	void printMarquee();
	void printCurrentCommand();
	void toggleCursorVisibility();
	void printCommandHistory();
};

