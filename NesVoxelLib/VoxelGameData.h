#pragma once

#include <vector>
#include "VoxelUtil.h"

const int spriteWidth = 8;
const int spriteHeight = 8;
const int spriteDepth = 32;
const int spriteSize = 8 * 8 * 32;

enum VoxelSide { left, right, top, bottom, front, back };

struct Voxel {
	int color;
	bool smooth;
};

class VoxelSprite {
public:
	VoxelSprite();
	Voxel voxels[2048];
	int zPosition;
	bool matchUp;
	bool matchDown;
	bool matchLeft;
	bool matchRight;
	VoxelMesh *mesh;
	void buildMesh();
	void randomizeSprite();
	void setVoxel(int, int, int, int);

private:
	Voxel getVoxel(int, int, int);
	void buildSide(std::vector<ColorVertex> &vertices, int x, int y, int z, int color, VoxelSide side);
	void clearVoxel();
};

class VoxelGameData {
public:
	VoxelGameData(int totalSprites, int ppuBankSize);
	int ppuBankSize;
	int totalSprites;
	std::vector<VoxelSprite> sprites;
	void buildAllMeshes();
private:
};