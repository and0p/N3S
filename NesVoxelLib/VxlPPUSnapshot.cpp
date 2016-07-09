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
		for (int i = 0; i < 64; i++)
		{
			unsigned char *spritePtr = (unsigned char*)(&rawPPU->oam[0] + (i*4));
			sprites.push_back(buildSprite(spritePtr));
		}
		background.addQuadrant((char*)&rawPPU->nameTables[0]);
		background.addQuadrant((char*)&rawPPU->nameTables[1]);
		background.addQuadrant((char*)&rawPPU->nameTables[2]);
		background.addQuadrant((char*)&rawPPU->nameTables[3]);
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

void Background::addQuadrant(char * data)
{
	NameTableQuadrant quadrant = NameTableQuadrant();
	for (int i = 0; i < 960; i++) {
		quadrant.tiles[i].tile = *(data + i);
	}
	quadrants.push_back(quadrant);
}