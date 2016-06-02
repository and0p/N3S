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

struct VxlRawPPU {
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


inline void VxlRawPPU::writeScrollValue(int v, int scanline) {
	// See if we need to add a new scroll state
	if (scrollWrites % 2 == 0)
	{
		scrollStates.push_back({ v, 0, scanline });
	}
	else
	{
		scrollStates.back().y = v;
	}
	// Increment writes
	scrollWrites++;
}