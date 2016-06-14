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

NameTableWrapper::NameTableWrapper(unsigned char * data)
{
	update(data);
}

void NameTableWrapper::update(unsigned char * data)
{
	for (int i = 0; i < 960; i++) {
		tiles[i].tile = *(data + i);
	}
}


VxlPPUSnapshot::VxlPPUSnapshot(VxlRawPPU * rawPPU)
{
		for (int i = 0; i < 64; i++) {
			unsigned char *spritePtr = (unsigned char*)(&rawPPU->oam[0] + (i*4));
			sprites.push_back(buildSprite(spritePtr));
		}
		nameTables->push_back(NameTableWrapper((unsigned char*)&rawPPU->nameTables[0]));
		nameTables->push_back(NameTableWrapper((unsigned char*)&rawPPU->nameTables[1]));
		// scrollStates = rawPPU->scrollStates;
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
	ptr += 2;
	sprite.x = *ptr;
	return sprite;
}

int VxlPPUSnapshot::getTileAddress(unsigned char byte)
{
	return (byte >> 1);
}

