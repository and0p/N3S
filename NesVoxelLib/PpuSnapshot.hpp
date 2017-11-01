#pragma once

#include <vector>
#include "N3sRawPpu.h"
#include "N3sPalette.hpp"
#include "N3sMath.hpp"
#include "Common.hpp"

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
	bool emphasizeRed;				// Red and green swapped on PAL and Dendy
	bool emphasizeGreen;			// Red and green swapped on PAL and Dendy
	bool emphasizeBlue;
};

struct OamSprite {
	unsigned char x;
	unsigned char y;
	bool spriteBank;
	unsigned int tile;
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

static enum MirrorType { vertical, horizontal, single, full, diagonal };

class Background {
public:
	int mirrorLayouts[5][4] = { 
		{ 0, 1, 0, 1},
		{ 0, 0, 1, 1 },
		{ 0, 0, 0, 0 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 1, 0 }
	};
	int mirrorPositions[5][4][2] = {
		{ { 0, 0 },{ 1, 0 },{ 0, 1 },{ 1, 1 } },
		{ { 0, 0 },{ 1, 0 },{ 0, 1 },{ 1, 1 } },
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
	void addQuadrant(char * data, bool nameTableSelection);
	Vector2D getTrueXY(int x, int y, int nametable);
private:
	std::vector<NameTableQuadrant> quadrants;
};

struct ScrollSection {
	int x = 0;
	int y = 0;
	int nameTable = 0;
	int top;
	int bottom;
	bool patternSelect;
};

class PpuSnapshot {
public:
	PpuSnapshot(N3sRawPpu *rawPPU);
	~PpuSnapshot();
	std::vector<OamSprite> sprites;
	int backgroundColor;
	Background background;
	N3sPalette palette;
	Ctrl ctrl;
	Mask mask;
	char *patternTable;
	std::vector<ScrollSection> scrollSections;
	std::map<int, bool> oamPatternSelect;
	int ppuScroll = 0;
	bool getOAMSelectAtScanline(int scanline);
	int getTrueOamTile(int s);
	int getTrueNTTile(int i);
	UINT registerOptions[REGISTER_OPTION_SIZE];
private:
	OamSprite buildSprite(unsigned char *ptr);
	static int getTileAddress(unsigned char byte);
};