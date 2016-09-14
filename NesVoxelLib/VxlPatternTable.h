#pragma once

#include <map>
#include <string>

const static int patternTableSize = 256;

class VxlVirtualPatternTable
{
public:
	VxlVirtualPatternTable();
	void load(int count, int divisor, char* data);
	void update(char* patternTable);
	int getTrueTileIndex(int tile);
	int trueSectionIndices[256];
	char* dataPointer;
	int tableDivisor;
	int tileCount;
	int sectionSize;
	int sectionByteWidth;
	int hashComplexity = 4;
	void reset();
private:
	std::map<std::string, int> map;
	std::string getPatternSectionHash(char* data);
};