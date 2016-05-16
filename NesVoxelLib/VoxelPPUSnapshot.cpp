#include "stdafx.h"
#include "VoxelPPUSnapshot.h"

VoxelPPUSnapshot::VoxelPPUSnapshot(const void *vram): nameTables(new std::vector<NameTable>)
{
	unsigned char *vramStart = (unsigned char*)vram;
	for (int i = 0; i < 64; i++) {
		unsigned char *spritePtr = vramStart + (i*4);
		sprites.push_back(buildSprite(spritePtr));
	}
	nameTables->push_back(NameTable(vramStart + 32 + 256));
	nameTables->push_back(NameTable(vramStart + 32 + 256 + 1024));
}

VoxelPPUSnapshot::~VoxelPPUSnapshot()
{
}

OamSprite VoxelPPUSnapshot::buildSprite(unsigned char* ptr)
{
	OamSprite sprite;
	sprite.y = *ptr;
	ptr += 1;
	sprite.tile = *ptr;
	ptr += 2;
	sprite.x = *ptr;
	return sprite;
}

int VoxelPPUSnapshot::getTileAddress(unsigned char byte)
{
	return (byte >> 1);
}

NameTable::NameTable(unsigned char * data)
{
	update(data);
	int test = 0;
}

void NameTable::update(unsigned char * data)
{
	for (int i = 0; i < 960; i++) {
		tiles[i].tile = *(data + i);
	}
}
