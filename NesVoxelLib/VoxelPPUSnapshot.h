#pragma once

#define PALETTE_SIZE	32;
#define OAM_SIZE		256;
#define	TABLE_SIZE		2048;
#define NST_FORCE_INLINE inline
#define NST_SINGLE_CALL __forceinline

#include <vector>

struct Scroll
{
	enum
	{
		X_TILE = 0x001F,
		Y_TILE = 0x03E0,
		Y_FINE = 0x7000,
		LOW = 0x00FF,
		HIGH = 0xFF00,
		NAME = 0x0C00,
		NAME_LOW = 0x0400,
		NAME_HIGH = 0x0800
	};

	NST_FORCE_INLINE void ClockX();
	NST_SINGLE_CALL  void ResetX();
	NST_SINGLE_CALL  void ClockY();

	unsigned int address;
	unsigned int toggle;
	unsigned int latch;
	unsigned int xFine;
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

class NameTable {
public:
	NameTable(unsigned char * data);
	void update(unsigned char * data);
	NameTableTile tiles[960];
};

class VoxelPPUSnapshot {
public:
	VoxelPPUSnapshot(const void *vram);
	~VoxelPPUSnapshot();
	std::vector<OamSprite> sprites;
	std::shared_ptr<std::vector<NameTable>> nameTables;
	int ppuScroll;
private:
	OamSprite buildSprite(unsigned char *ptr);
	static int getTileAddress(unsigned char byte);
};