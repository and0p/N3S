#pragma once

#include <vector>
#include "VxlRawPPU.h"

struct Ctrl {
	bool spriteNameTable;
	bool backgroundNameTable;
	bool spriteSize16x8;
};

struct Mask {
	bool greyscale;
	bool renderSprites;
	bool renderBackground;
	bool renderSpritesLeft8;
	bool renderBackgroundLeft8;
	bool emphasizeRed;				// Red and green swaped on PAL and Dendy
	bool emphasizeGreen;
	bool emphasizeBlue;
};

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

struct NameTableQuadrant {
	NameTableTile tiles[960];
};

struct IndividualPalette {
	int colors[3];
};

struct FullPalette {
	IndividualPalette palettes[8];
};

class Background {
public:
	static enum MirrorType { vertical, horizontal, single, full, diagonal };
	int mirrorLayouts[5][4] = { 
		{ 0, 1, 0, 1},
		{ 0, 0, 1, 1 },
		{ 0, 0, 0, 0 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 1, 0 }
	};
	int mirrorPositions[5][4][2] = {
		{ { 0, 0 },{ 1, 0 },{ 0, 0 },{ 0, 0 } },
		{ { 0, 0 },{ 0, 1 },{ 0, 0 },{ 0, 0 } },
		{ { 0, 0 },{ 0, 0 },{ 0, 0 },{ 0, 0 } },
		{ { 0, 0 },{ 1, 0 },{ 0, 1 },{ 1, 1 } },
		{ { 0, 0 },{ 1, 0 },{ 0, 0 },{ 0, 0 } },
	};
	int mirrorSizes[5][2] = {
		{ 64, 30 },
		{ 32, 60 },
		{ 32, 30 },
		{ 64, 60 },
		{ 64, 60 }
	};
	MirrorType mirrorType = vertical;
	NameTableTile getTile(int x, int y, int nametable);
	void addQuadrant(char * data);
private:
	std::vector<NameTableQuadrant> quadrants;
};

struct ScrollSection {
	int x;
	int y;
	int nameTable;
	int top;
	int bottom;
};

class VxlPPUSnapshot {
public:
	VxlPPUSnapshot(VxlRawPPU *rawPPU);
	~VxlPPUSnapshot();
	std::vector<OamSprite> sprites;
	int backgroundColor;
	Background background;
	FullPalette palette;
	Ctrl ctrl;
	Mask mask;
	char *patternTable;
	std::vector<ScrollSection> scrollSections;
	int ppuScroll = 0;
private:
	OamSprite buildSprite(unsigned char *ptr);
	static int getTileAddress(unsigned char byte);
};