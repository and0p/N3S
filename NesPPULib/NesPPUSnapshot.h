#pragma once

#include <vector>

struct PatternTable {
	char data[4098];
};

struct NameTable {
	char data[1024];
};

struct ScrollState {
	int x;
	int y;
	int scanline;
};

class VxlRawPPU {
public:
	char ctrl;
	char mask;
	char status;
	char palette[32];
	char oam[256];
	PatternTable patternTables[2];
	NameTable nameTables[4];
	std::vector<ScrollState> scrollStates;
	void writeScrollValue(int v, int scanline);
private:
	int scrollWrites = 0;
};