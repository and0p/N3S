#include "stdafx.h"
#include "VxlPatternTable.h"

VxlVirtualPatternTable::VxlVirtualPatternTable()
{
}

void VxlVirtualPatternTable::load(int count, int divisor, char * data)
{
	tileCount = count;
	tableDivisor = divisor;
	dataPointer = data;
	// Count how many sections we have in total
	sectionSize = 512 / tableDivisor;
	sectionByteWidth = sectionSize * 16;
	int sectionCount = tileCount / sectionSize;
	// Create a hash for each, with more complicated hashes in case of collision
	bool hashCollision = true;
	while (hashCollision)
	{
		bool hashCollisionThisIteration = false;
		map.clear();
		for (int i = 0; i < sectionCount && !hashCollisionThisIteration; i++)
		{
			int trueIndex = sectionSize * i;
			std::string hash = getPatternSectionHash(dataPointer + (trueIndex * 16));
			// Check for collisions, but give up if we're already sampling a lot of data
			if (map.count(hash) > 0 && hashComplexity <= 16)
				hashCollisionThisIteration = true;
			map[hash] = trueIndex;
		}
		// If we have a collision after 8 levels of complexity, just ignore
		if(hashCollisionThisIteration)
			hashComplexity += 1;
		hashCollision = hashCollisionThisIteration;
	}
}

void VxlVirtualPatternTable::update(char * patternTable)
{
	// Refresh each hash
	for (int i = 0; i < tableDivisor; i++)
	{
		std::string hash = getPatternSectionHash(patternTable + (i * sectionByteWidth));
		if (map.count(hash) < 1)
			trueSectionIndices[i] = 0;
		else
			trueSectionIndices[i] = map.at(hash);
	}
}

int VxlVirtualPatternTable::getTrueTileIndex(int tile)
{
	// Get which mapped section of pattern RAM this tile falls under
	int section = floor(tile / sectionSize);
	// Get which tile within that section this would be (tile 260 if section size is 256 would be 4)
	int tileWithinSection = tile % sectionSize;
	return trueSectionIndices[section] + tileWithinSection;
}

void VxlVirtualPatternTable::reset()
{
	map.clear();
}

std::string VxlVirtualPatternTable::getPatternSectionHash(char *data)
{
	int size = 512 / tableDivisor;
	int byteWidth = size * 16;
	std::string hash = "";
	for (int i = 0; i < hashComplexity; i++)
	{
		for (int x = 0; x < 4; x++)
		{
			signed char c = *(data + ((byteWidth / 4) * x) + i) % byteWidth;
			hash += c;
		}
	}
	return hash;
}