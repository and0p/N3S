#pragma once
#include <time.h>
#include "Overlay.hpp"
#include <vector>

using namespace std;

const int messageDuration = 3;	// Console message duration in seconds
const int consoleScale = 3;

class ConsoleLine
{
public:
	ConsoleLine(string line);
	bool stillAlive();
	string line;
private:
	clock_t creationTime;
};

class N3sConsole {
public:
	static void init();
	static void writeLine(string line);
	static void update();
	static void render();
private:
	static unique_ptr<vector<ConsoleLine>> lines;
};