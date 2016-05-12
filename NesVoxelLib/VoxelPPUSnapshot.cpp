#include "stdafx.h"
#include "VoxelPPUSnapshot.h"

VoxelPPUSnapshot::VoxelPPUSnapshot(const void *vram)
{
	unsigned char *oamStart = (unsigned char*)vram;
	for (int i = 0; i < 64; i++) {
		unsigned char *spritePtr = oamStart + (i*4);
		sprites[i] = buildSprite(spritePtr);
	}
}

VoxelPPUSnapshot::~VoxelPPUSnapshot()
{
}

std::shared_ptr<OamSprite> VoxelPPUSnapshot::buildSprite(unsigned char* ptr)
{
	std::shared_ptr<OamSprite> sprite(new OamSprite());
	sprite->y = *ptr;
	ptr += 3;
	sprite->x = *ptr;
	return sprite;
}