#pragma once

#include <vector>
#include "VxlRawPPU.h"

struct OamSprite {
	unsigned char x;
	unsigned char y;
	bool spriteBank;
	unsigned char tile;
	bool hFlip;
	bool vFlip;
	bool priority;
	unsigned char palette;
};

struct NameTableTile {
	int tile;
	int palette;
};

class NameTableWrapper {
public:
	NameTableWrapper(unsigned char * data);
	void update(unsigned char * data);
	NameTableTile tiles[960];
};

class VxlPPUSnapshot {
public:
	VxlPPUSnapshot(VxlRawPPU *rawPPU);
	~VxlPPUSnapshot();
	std::vector<OamSprite> sprites;
	std::shared_ptr<std::vector<NameTableWrapper>> nameTables;
	std::vector<ScrollState> scrollStates;
	int ppuScroll = 0;
private:
	OamSprite buildSprite(unsigned char *ptr);
	static int getTileAddress(unsigned char byte);
};