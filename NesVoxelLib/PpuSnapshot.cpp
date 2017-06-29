#include "stdafx.h"
#include "PpuSnapshot.hpp"
#include "N3s3d.hpp"

//PpuSnapshot::PpuSnapshot(const void *vram): nameTables(new std::vector<NameTable>)
//{
//	unsigned char *vramStart = (unsigned char*)vram;
//	for (int i = 0; i < 64; i++) {
//		unsigned char *spritePtr = vramStart + (i*4);
//		sprites.push_back(buildSprite(spritePtr));
//	}
//	nameTables->push_back(NameTable(vramStart + 32 + 256));
//	nameTables->push_back(NameTable(vramStart + 32 + 256 + 1024));
//	// Scroll *s = (Scroll*)(vramStart + 32 + 256 + 2048);
//	// ppuScroll = s->xFine;
//	int test = 0;
//}

PpuSnapshot::PpuSnapshot(N3sRawPpu * rawPPU)
{
	// Copy register options
	ctrl.spriteNameTable = getBit(rawPPU->ctrl, 3);
	ctrl.backgroundNameTable = getBit(rawPPU->ctrl, 4);
	ctrl.spriteSize16x8 = getBit(rawPPU->ctrl, 5);
	mask.greyscale = getBit(rawPPU->mask, 0);
	mask.renderBackgroundLeft8 = getBit(rawPPU->mask, 1);
	mask.renderSpritesLeft8 = getBit(rawPPU->mask, 2);
	mask.renderBackground = getBit(rawPPU->mask, 3);
	mask.renderSprites = getBit(rawPPU->mask, 4);
	mask.emphasizeRed = getBit(rawPPU->mask, 5);
	mask.emphasizeGreen = getBit(rawPPU->mask, 6);
	mask.emphasizeBlue = getBit(rawPPU->mask, 7);
	// Build sprites
	for (int i = 0; i < 64; i++)
	{
		unsigned char *spritePtr = (unsigned char*)(&rawPPU->oam[0] + (i*4));
		sprites.push_back(buildSprite(spritePtr));
	}
	// Grab pattern table
	patternTable = &rawPPU->patternTable.data[0];
	// Grab background quadrants
	background.addQuadrant((char*)&rawPPU->nameTables[0], ctrl.backgroundNameTable);
	background.addQuadrant((char*)&rawPPU->nameTables[1], ctrl.backgroundNameTable);
	background.addQuadrant((char*)&rawPPU->nameTables[2], ctrl.backgroundNameTable);
	background.addQuadrant((char*)&rawPPU->nameTables[3], ctrl.backgroundNameTable);
	// Set mirroring type
	switch (rawPPU->mirroring)
	{
	case 12:
		background.mirrorType = horizontal;
		break;
	case 10:
		background.mirrorType = vertical;
		break;
	case 0:
		background.mirrorType = vertical;
		break;
	case 1:
		background.mirrorType = vertical;
		break;
	}
	// Copy background color
	palette.backgroundColorIndex = (int)rawPPU->palette[0];
	// Copy palette
	for (int p = 0; p < 8; p++)
	{
		for (int c = 0; c < 3; c++)
		{
			palette.colorIndices[(p * 3) + c] = (int)rawPPU->palette[(p * 4) + 1 + c];
		}
	}
	// Create variables to track most recent scroll variables
	int lastX = 0;
	int lastY = 0;
	int lastNametable = 0;
	int lastScanline = 0;
	bool lastPatternSelect = false;
	bool sectionAdded = false;
	// Create "sections" of screen where scroll has changed
	for (auto kv : rawPPU->scrollSnapshots)
	{
		// Read current values
		int currentX = kv.second.getTrueX();
		int currentY = kv.second.getTrueY();
		int currentNametable = kv.second.v.nametable;
		bool currentPatternSelect = kv.second.patternSelect;
		// Measure if the change in Y matches change in scanline, which would indicate Y wasn't changed in way we'll see
		int currentScanline = kv.first;
		int scanlineDelta = currentScanline - lastScanline;
		int yDelta = currentY - lastY;
		// Push to list only if different from last snapshot and Y change hasn't matched scanline change
		if (!sectionAdded || currentX != lastX || scanlineDelta != yDelta || currentNametable != lastNametable || currentPatternSelect != lastPatternSelect)
		{
			ScrollSection s = ScrollSection();
			s.top = currentScanline;
			s.x = currentX;
			s.y = currentY;
			s.nameTable = currentNametable;
			s.patternSelect = currentPatternSelect;
			scrollSections.push_back(s);
			lastX = currentX;
			lastY = currentY;
			lastNametable = currentNametable;
			lastScanline = currentScanline;
			lastPatternSelect = currentPatternSelect;
			sectionAdded = true;
		}
	}
	// Create sections where OAM pattern is different
	oamPatternSelect = rawPPU->oamPatternSelect;
	// Make sure first and last scroll sections take up the full screen
	if (scrollSections.size() > 0)
	{
		scrollSections.front().top = 0;
		scrollSections.back().bottom = 239;
		// Change the bottom of all scroll sections to be top of next if needed
		if (scrollSections.size() > 1)
		{
			for (int i = 0; i < scrollSections.size()-1; i++)
			{
				scrollSections[i].bottom = scrollSections[i + 1].top - 1;
			}
		}
	}
}

PpuSnapshot::~PpuSnapshot()
{
}

bool PpuSnapshot::getOAMSelectAtScanline(int scanline)
{
	// Find which scroll section this sits in
	bool passed = false;
	bool value = false;
	for (auto kv : oamPatternSelect)
	{
		if (kv.first <= scanline)
		{
			// Set that this might contain the value we're looking for
			passed = true;
			value = kv.second;
		}
		// When we know for sure, return it
		if (kv.first > scanline && passed)
			return value;
	}
	// This just makes sure we return the last value
	return value;
}

OamSprite PpuSnapshot::buildSprite(unsigned char* ptr)
{
	OamSprite sprite;
	sprite.y = *ptr;
	ptr += 1;
	sprite.tile = *ptr + (ctrl.spriteNameTable * 256);
	ptr += 1;
	sprite.hFlip = (*ptr >> 6) & 1;
	sprite.vFlip = (*ptr >> 7) & 1;
	sprite.palette = (*ptr & 3) + 4;
	sprite.priority = (*ptr >> 5) & 1;
	ptr += 1;
	sprite.x = *ptr;
	return sprite;
}

int PpuSnapshot::getTileAddress(unsigned char byte)
{
	return (byte >> 1);
}

int getQuadrant(int x, int y)
{
	if (x < 32)
	{
		if (y < 30)
		{
			return 0;
		}
		else
		{
			return 2;
		}
	}
	else
	{
		if (y < 30)
		{
			return 1;
		}
		else
		{
			return 3;
		}
	}
}

NameTableTile Background::getTile(int x, int y, int nametable)
{
	// Get true X,Y
	if (mirrorPositions[mirrorType][nametable][0])
	{
		x += 32;
	}
	if (mirrorPositions[mirrorType][nametable][1])
	{
		y += 30;
	}
	if (x > 64)
		int test = 0;
	// If this new X,Y is out of bounds, make it wrap around back from the top-left
	x %= mirrorSizes[mirrorType][0];
	y %= mirrorSizes[mirrorType][1];
	// Find which quadrant this ultimately falls into
	int quadrant = getQuadrant(x, y);
	// Reset X&Y to local quadrant coordinates
	x %= 32;
	y %= 30;
	return quadrants[mirrorLayouts[mirrorType][quadrant]].tiles[(y * 32) + x];
}

void updatePaletteFor16x16(int index, int palette, NameTableQuadrant *quadrant)
{
	quadrant->tiles[index].palette = palette;
	quadrant->tiles[index + 1].palette = palette;
	quadrant->tiles[index + 32].palette = palette;
	quadrant->tiles[index + 33].palette = palette;
}

void Background::addQuadrant(char * data, bool nameTableSelection)
{
	NameTableQuadrant quadrant = NameTableQuadrant();
	// Get tiles from memory
	for (int i = 0; i < 960; i++)
	{
		int tile = *(data + i);
		if (tile < 0)
			tile += 256;
		quadrant.tiles[i].tile = tile;
	}
	// Get tile palettes from memory
	for (int i = 0; i < 64; i++)
	{
		int indexColumn = (i * 4) % 32;
		int indexRow = floor(i / 8);
		int index = indexColumn + (indexRow * 128);
		char palette = *(data + 960 + i);
		int testP = (palette) & 3;
		// Insert palette into the 4 (2x2) tiles in the section the attribute table is addressing
		updatePaletteFor16x16(index, palette & 3, &quadrant);
		updatePaletteFor16x16(index + 2, (palette >> 2) & 3, &quadrant);
		// Skip insert second sub-row if bottom row, which is only 1 high
		if (indexRow != 7)
		{
			updatePaletteFor16x16(index + 64, (palette >> 4) & 3, &quadrant);
			updatePaletteFor16x16(index + 66, (palette >> 6) & 3, &quadrant);
		}
	}
	quadrants.push_back(quadrant);
}

int PpuSnapshot::getTrueOamTile(int s)
{
	int tile = sprites[s].tile;
	int y = sprites[s].y;
	if (ctrl.spriteSize16x8)
	{
		// In 8x16, pattern table selection is specified by first bit.
		// Since you can only select even (in base 0) tiles it is free for this purpose.
		// Actual tile selection is bits 1-7.
		int patternSelection = tile & 1;
		// Nestopia tends to give tiles by absolute value, but seemingly only some of the time?
		// So we need to correct to be sure
		if (patternSelection && tile < 256)
			tile += 256;
		else if (!patternSelection && tile > 255)
			tile -= 256;
		if (patternSelection)
			tile--;
	}
	else
	{
		// Select tile based on pattern table in CTRL register
 		if (tile > 255 && !getOAMSelectAtScanline(y))
		{
			tile -= 256;
		}
		else if (tile < 256 && getOAMSelectAtScanline(y))
		{
			tile += 256;
		}
	}
	return tile;
}

int PpuSnapshot::getTrueNTTile(int i)
{
	Vector2D v = unwrapArrayIndex(i, 64);
	int tile = background.getTile(v.x, v.y, 0).tile;
	if (ctrl.backgroundNameTable)
		return tile + 256;
	else
		return tile;
}
