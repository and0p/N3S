#pragma once

#include <vector>
#include "VxlUtil.h"
#include <unordered_map>
#include <string>
#include "json.hpp"
#include "VxlHash.hpp"
#include <unordered_set>

using json = nlohmann::json;
using namespace std;

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

class SpriteGroup {
public:
	SpriteGroup(string hash, int size);
	string hash;
	vector<shared_ptr<VoxelSprite>> sprites;
};

class VoxelSprite {
public:
	VoxelSprite();
	Voxel voxels[2048];
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
	void renderPartial(int x, int y, int palette, int xOffset, int width, int yOffset, int height, bool mirrorH, bool mirrorV);
	json getJSON();
private:
	Voxel getVoxel(int, int, int);
	static void buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side);
	void clear();
};

class VoxelGameData {
public:
	VoxelGameData(char * data, int spriteGroupSize);
	VoxelGameData(json json);
	char * chrData; // Any need for this?
	int totalSprites;
	int spriteGroupSize;
	int totalSpriteGroups;
	vector<shared_ptr<SpriteGroup>> spriteGroups;
	unordered_map<string, shared_ptr<SpriteGroup>> spriteGroupsByHash;
	unordered_map<int, VoxelSprite> sprites;
	unordered_map<int, VoxelMesh> meshes;
	vector<BitmapSprite> bitmaps;		// | Does any of this need to be stored after loading?
	void createSpritesFromBitmaps();	// | Doesn't seem like it...
	void buildAllMeshes();
	void grabBitmapSprites(const void * gameData);
	static VoxelMesh getSharedMesh(int zArray[32]);
	static void releaseSharedMesh(string hash);
	CartridgeInfo cartridgeInfo;
	void unload();
	void exportToJSON();
private:
	static unordered_map<string, SharedMesh> sharedMeshes;
};

CartridgeInfo getCartidgeInfo(char * data);