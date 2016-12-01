#pragma once
#include <memory>
#include <unordered_map>
#include "json.hpp"
#include "VxlUtil.h"
#include <unordered_set>
#include "VxlPPUSnapshot.h"

using json = nlohmann::json;
using namespace std;

const int spriteWidth = 8;
const int spriteHeight = 8;
const int spriteDepth = 32;
const int spriteSize = 8 * 8 * 32;

const char hexCodes[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
const char intChars[10] = { '0','1','2','3','4','5','6','7','8','9' };

enum VoxelSide { left, right, top, bottom, front, back };

struct SharedMesh {
	VoxelMesh mesh;
	int referenceCount;
};

struct Voxel {
	int8_t color;
};

class VoxelCollection
{
public:
	VoxelCollection();
	VoxelCollection(char* sprite);
	VoxelCollection(json j);
	Voxel getVoxel(int x, int y, int z);
	void setVoxel(int x, int y, int z, Voxel v);
	void clear();
	Voxel voxels[spriteSize];
	json getJSON();
};

class SpriteMesh
{
public:
	SpriteMesh(char* spriteData);
	SpriteMesh(json j);
	int id;
	unique_ptr<VoxelCollection> voxels;
	VoxelMesh mesh;
	VoxelMesh zMeshes[64];
	void setVoxel(int x, int y, int z, int color);
	bool buildMesh();
	bool meshExists = false;
	void render(int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop);
	json getJSON();
private:
	void buildZMeshes();
	// void rebuildZMesh(int x, int y);
};

class VirtualSprite
{
public:
	VirtualSprite();
	VirtualSprite(string chrData, shared_ptr<SpriteMesh> mesh);
	VirtualSprite(json j, map<int, shared_ptr<SpriteMesh>> meshes);
	void renderOAM(shared_ptr<VxlPPUSnapshot> snapshot, int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop);
	void renderNametable(shared_ptr<VxlPPUSnapshot> snapshot, int x, int y, int palette, int nametableX, int nametableY, Crop crop);
	json getJSON();
	int id;
	vector<int> appearancesInRomChr;	// Where does this sprite appear in CHR data? For reference.
	string chrData;
private:
	string description = "";
	shared_ptr<SpriteMesh> defaultMesh;
	string serializeChrDataAsText();
	void getChrStringFromText(string s);
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
	GameData(char* data, json j);
	map<int, shared_ptr<VirtualSprite>> sprites;
	map<string, shared_ptr<VirtualSprite>> spritesByChrData;
	map<int, shared_ptr<SpriteMesh>> meshes;
	shared_ptr<VirtualSprite> getSpriteByChrData(char* data);
	RomInfo romInfo;
	int totalSprites;
	static VoxelMesh getSharedMesh(int zArray[32]);
	static void releaseSharedMesh(string hash);
	void unload();
	string getJSON();
private:
	static unordered_map<string, SharedMesh> sharedMeshes;
};

static VoxelMesh buildZMesh(int zArray[32]);
static void buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side);
void setVoxelColors(char a, char b, Voxel* row);
bool getBitLeftSide(char byte, int position);
string getPaddedStringFromInt(int i, int length);
int charToInt(char c);