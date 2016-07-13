#include "stdafx.h"
#include "VxlPatternTable.h"

VxlVirtualPatternTable::VxlVirtualPatternTable(int count, int divisor, char * data)
{
	tileCount = count;
	tableDivisor = divisor;
	dataPointer = data;
	// Count how many sections we have in total
	int sectionSize = 256 / tableDivisor;
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
			std::string hash = getPatternSectionHash(dataPointer + trueIndex);
			// Check for collisions
			if (map.count(hash) > 0)
				hashCollisionThisIteration = true;
			map[hash] = trueIndex;
		}
		hashComplexity += 1;
		// If we have a collision after 8 levels of complexity, just ignore
		if (hashComplexity > 8)
			hashCollisionThisIteration = false;
		hashCollision = hashCollisionThisIteration;
	}
}

void VxlVirtualPatternTable::update(char * patternTable)
{
	int sectionSize = 256 / tableDivisor;
	// Refresh each hash
	for (int i = 0; i < tableDivisor; i++)
	{
		trueIndices[i] = map[getPatternSectionHash(patternTable + (sectionSize * i))];
	}
}

std::string VxlVirtualPatternTable::getPatternSectionHash(char *data)
{
	int size = 256 / tableDivisor;
	int byteWidth = size * 16;
	std::string hash = "";
	for (int i = 0; i < hashComplexity; i++)
	{
		hash += *(data + (byteWidth / 4) + i) % byteWidth;
	}
	return hash;
}