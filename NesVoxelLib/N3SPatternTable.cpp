#include "stdafx.h"
#include "N3SPatternTable.hpp"

VirtualPatternTable::VirtualPatternTable(shared_ptr<GameData> gameData): gameData(gameData)
{
	for (int i = 0; i < 512; i++)
		sprites[i] = 0;
}

void VirtualPatternTable::update(char * patternTable)
{
	vector<int> updatedSprites;
	// If this is the first frame, all sprites need to be updated
	if (firstFrame)
	{
		firstFrame = false;
		for (int i = 0; i < 512; i++)
			updatedSprites.push_back(i);
		copyBuffer(patternTable);
	}
	else
	{
		int position = 0;
		// Loop through remote pattern table
		for (int i = 0; i < 512; i++)
		{
			// Loop through 16 bytes of each sprite
			for (int s = 0; s < 16; s++)
			{
				position = (i * 16) + s;
				// If sprite any byte has changed, add to list of changed sprites and set buffer as changed
				if (*(patternTable + position) != patternTableCache[position])
				{
					updatedSprites.push_back(i);
					bufferHasChanged = true;
					break;
				}
			}
		}
	}
	// Copy over buffer if it changed within the NES
	if (bufferHasChanged)
	{
		copyBuffer(patternTable);
		bufferHasChanged = false;
	}
	// Loop through changed sprites, and use hash to grab integer ID of N3S virtual sprite
	for each(int s in updatedSprites)
	{
		sprites[s] = gameData->getSpriteByChrData(&patternTableCache[s * 16]);
	}
}

void VirtualPatternTable::copyBuffer(char * patternTable)
{
	memcpy(&patternTableCache, patternTable, 8192);
}

shared_ptr<VirtualSprite> VirtualPatternTable::getSprite(int i)
{
	return sprites[i];
}

void VirtualPatternTable::reset()
{
	for (int i = 0; i < 512; i++)
		sprites[i] = 0;
	bufferHasChanged = false;
	firstFrame = true;
}
