#pragma once
#include <memory>
#include <unordered_map>
#include "json.hpp"
#include "VxlUtil.h"
#include <unordered_set>

using json = nlohmann::json;
using namespace std;

const int spriteWidth = 8;
const int spriteHeight = 8;
const int spriteDepth = 32;
const int spriteSize = 8 * 8 * 32;

enum VoxelSide { left, right, top, bottom, front, back };

struct SharedMesh {
	VoxelMesh mesh;
	int referenceCount;
};

struct Voxel {
	int8_t color;
};

class VirtualSprite
{
public:
	virtual json getJSON() = 0;
};

class StaticSprite : public VirtualSprite
{
public:
	StaticSprite(int mesh);
	int mesh;
	json getJSON();
};

class DynamicSprite
{
public:
	// json getJSON();
};

class VoxelCollection
{
public:
	VoxelCollection();
	VoxelCollection(char* sprite);
	Voxel getVoxel(int x, int y, int z);
	void setVoxel(int x, int y, int z, Voxel v);
	void clear();
	Voxel voxels[spriteSize];
};

class SpriteMesh
{
public:
	SpriteMesh(char* spriteData);
	SpriteMesh(json data, bool edit);
	unique_ptr<VoxelCollection> voxels;
	VoxelMesh mesh;
	VoxelMesh zMeshes[64];
	void setVoxel(int x, int y, int z, int color);
	bool buildMesh();
private:
	void buildZMeshes();
};

struct RomInfo
{
	int prgSize;
	int chrSize;
	int mapper;
	bool trainer;
	bool playChoice10;
	bool PAL;
};

class GameData
{
public:
	GameData(char* data);
	GameData(json data);
	map<int, VirtualSprite> sprites;
	map<int, SpriteMesh> meshes;
	RomInfo romInfo;
	int totalSprites;
	static VoxelMesh getSharedMesh(int zArray[32]);
	static void releaseSharedMesh(string hash);
	void unload();
	void getJSON();
private:
	static unordered_map<string, SharedMesh> sharedMeshes;
};

static VoxelMesh buildZMesh(int zArray[32]);
static void buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side);
void setVoxelColors(char a, char b, Voxel* row);
bool getBit(char byte, int position);
bool getBitLeftSide(char byte, int position);