#include "stdafx.h"
#include "VxlPPUSnapshot.h"

//VxlPPUSnapshot::VxlPPUSnapshot(const void *vram): nameTables(new std::vector<NameTable>)
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

VxlPPUSnapshot::VxlPPUSnapshot(VxlRawPPU * rawPPU)
{
	// Copy register options
	ctrl.spriteNameTable = (rawPPU->ctrl >> 3) & 1;
	ctrl.backgroundNameTable = (rawPPU->ctrl >> 4) & 1;
	ctrl.spriteSize16x8 = (rawPPU->ctrl >> 5) & 1;
	mask.greyscale = rawPPU->mask & 1;
	mask.renderBackgroundLeft8 = (rawPPU->mask >> 1) & 1;
	mask.renderSpritesLeft8 = (rawPPU->mask >> 2) & 1;
	mask.renderBackground = (rawPPU->mask >> 3) & 1;
	mask.renderSprites = (rawPPU->mask >> 4) & 1;
	mask.emphasizeRed = (rawPPU->mask >> 5) & 1;
	mask.emphasizeGreen = (rawPPU->mask >> 6) & 1;
	mask.emphasizeBlue = (rawPPU->mask >> 7) & 1;
	// Build sprites
	for (int i = 0; i < 64; i++)
	{
		unsigned char *spritePtr = (unsigned char*)(&rawPPU->oam[0] + (i*4));
		sprites.push_back(buildSprite(spritePtr));
	}
	// Grab pattern table
	patternTable = &rawPPU->patternTable.data[0];
	// Grab background quadrants
	background.addQuadrant((char*)&rawPPU->nameTables[0]);
	background.addQuadrant((char*)&rawPPU->nameTables[1]);
	background.addQuadrant((char*)&rawPPU->nameTables[2]);
	background.addQuadrant((char*)&rawPPU->nameTables[3]);
	// Copy palette
	for (int p = 0; p < 8; p++)
	{
		for (int c = 0; c < 3; c++)
		{
			palette.palettes[p].colors[c] = (int)rawPPU->palette[(p*4) + 1 + c];
		}
	}
	// Copy background color
	backgroundColor = (int)rawPPU->palette[0];
	// Create "sections" of screen where scroll has changed
	for (auto kv : rawPPU->scrollStates)
	{
		ScrollSection s = ScrollSection();
		s.top = kv.first;
		s.x = kv.second.x;
		s.y = kv.second.y;
		s.nameTable = kv.second.highOrderBit;
		scrollSections.push_back(s);
	}
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

VxlPPUSnapshot::~VxlPPUSnapshot()
{
}

OamSprite VxlPPUSnapshot::buildSprite(unsigned char* ptr)
{
	OamSprite sprite;
	sprite.y = *ptr;
	ptr += 1;
	sprite.tile = *ptr;
	ptr += 1;
	sprite.hFlip = (*ptr >> 6) & 1;
	sprite.vFlip = (*ptr >> 7) & 1;
	sprite.palette = (*ptr & 3) + 4;
	sprite.priority = (*ptr >> 5) & 1;
	ptr += 1;
	sprite.x = *ptr;
	return sprite;
}

int VxlPPUSnapshot::getTileAddress(unsigned char byte)
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
	if (quadrant != 0)
	{
		int hi = 0;
	}
	// Reset X&Y to local quadrant coordinates
	x %= 32;
	y %= 30;
	return quadrants[quadrant].tiles[(y * 32) + x];
}

void updatePaletteFor16x16(int index, int palette, NameTableQuadrant *quadrant)
{
	quadrant->tiles[index].palette = palette;
	quadrant->tiles[index + 1].palette = palette;
	quadrant->tiles[index + 32].palette = palette;
	quadrant->tiles[index + 33].palette = palette;
}

void Background::addQuadrant(char * data)
{
	NameTableQuadrant quadrant = NameTableQuadrant();
	// Get tiles from memory
	for (int i = 0; i < 960; i++)
	{
		quadrant.tiles[i].tile = *(data + i);
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