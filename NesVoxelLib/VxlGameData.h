#pragma once

#include <vector>
#include "VxlUtil.h"
#include <unordered_map>
#include <string>

const int spriteWidth = 8;
const int spriteHeight = 8;
const int spriteDepth = 32;
const int spriteSize = 8 * 8 * 32;

enum VoxelSide { left, right, top, bottom, front, back };
enum TVSystem { NTSC, PAL };

struct CartridgeInfo
{
	int prgSize;
	int chrSize;
	int mapper;
	bool trainer;
	bool playChoice10;
	bool PAL;
};

struct Voxel {
	int color;
};

class BitmapSprite {
public:
	BitmapSprite();
	BitmapSprite(char *data);
	~BitmapSprite();
	int pixels[64];
private:
	void addPixels(char a, char b, int row);
	static bool getBit(char byte, int bitNumber);
	static bool getBitLeftSide(char byte, int position);
};

class SharedMesh {
public:
	VoxelMesh mesh;
	int referenceCount;
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
	VoxelMesh mesh;
	VoxelMesh zMeshes[64];
	bool buildMesh();
	void buildZMeshes();
	static VoxelMesh buildZMesh(int zArray[32]);
	bool meshExists;
	void randomizeSprite();
	void setVoxel(int, int, int, int);
	void buildFromBitmapSprite(BitmapSprite bitmap);
	void render(int x, int y, int palette, bool mirrorH, bool mirrorV);
	void renderPartial(int x, int y, int palette, Sides offset, bool mirrorH, bool mirrorV);
private:
	Voxel getVoxel(int, int, int);
	static void buildSide(std::vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side);
	void clear();
};

class VoxelGameData {
public:
	VoxelGameData(char * data);
	char * chrData;
	int totalSprites;
	std::vector<VoxelSprite> sprites;
	std::vector<BitmapSprite> bitmaps;
	void createSpritesFromBitmaps();
	void buildAllMeshes();
	void grabBitmapSprites(const void * gameData);
	static VoxelMesh getSharedMesh(int zArray[32]);
	static void releaseSharedMesh(std::string hash);
	CartridgeInfo cartridgeInfo;
private:
	static std::unordered_map<std::string, SharedMesh> sharedMeshes;
};

CartridgeInfo getCartidgeInfo(char * data);