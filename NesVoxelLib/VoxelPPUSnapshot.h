#pragma once

#define PALETTE_SIZE	32;
#define OAM_SIZE		256;
#define	TABLE_SIZE		2048;

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

class VoxelPPUSnapshot {
public:
	VoxelPPUSnapshot(const void *vram);
	~VoxelPPUSnapshot();
	std::shared_ptr<OamSprite> sprites[64];
private:
	std::shared_ptr<OamSprite> buildSprite(unsigned char *ptr);
};