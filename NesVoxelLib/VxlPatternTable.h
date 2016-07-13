#pragma once

#include <map>
#include <string>

const static int patternTableSize = 256;

class VxlVirtualPatternTable
{
public:
	VxlVirtualPatternTable(int count, int divisor, char* data);
	void update(char* patternTable);
	int trueIndices[256];
	char* dataPointer;
	int tableDivisor;
	int tileCount;
	int hashComplexity = 4;
private:
	std::map<std::string, int> map;
	std::string getPatternSectionHash(char* data);
};