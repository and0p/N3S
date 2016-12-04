#pragma once
#include "GameData.hpp"

using namespace std;

class VirtualPatternTable
{
public:
	VirtualPatternTable(shared_ptr<GameData> gameData);
	void update(char* patternTable);
	void copyBuffer(char * patternTable);
	shared_ptr<VirtualSprite> getSprite(int i);
	void reset();
private:
	shared_ptr<GameData> gameData;
	shared_ptr<VirtualSprite> sprites[512];
	char patternTableCache[8192];
	bool bufferHasChanged = false;
	bool firstFrame = true;
};