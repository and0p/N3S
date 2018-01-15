#pragma once
#include <memory>
#include <unordered_map>
#include "json.hpp"
#include "N3s3d.hpp"
#include <unordered_set>
#include "PpuSnapshot.hpp"

using json = nlohmann::json;
using namespace std;

const int spriteWidth = 8;
const int spriteHeight = 8;
const int spriteDepth = 32;
const int spriteSize = 8 * 8 * 32;

const char hexCodes[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
const char intChars[10] = { '0','1','2','3','4','5','6','7','8','9' };

enum VoxelSide { left, right, top, bottom, front, back };

enum StencilGrouping { no_grouping, sameColor, continous, continous_samecolor, adjacent_samecolor };

enum DynamicParameter { 
	color_id,				// id (0-63) of color in slot	[slot, color]
	all_color_ids,			// id (0-63) of all colors, -1 if should ignore [color, color, color, bg_color]
	palette_num,			// Palette number of sprite 0-8
	screen_coordinates,		// X / Y of coordinates on screen, OAM by pixels and NT by grid position
	absolute_nametable,		// Absolute coordinates of NT
	relative_nametable,		// Sprite ID of nametable that is X/Y  [x distance, y distance, id]
	is_oam,					// Is OAM	[1 = true 0 = false]
	
};

enum DynamicComparison { equal, not_equal, greater, less, greater_or_equal, less_or_equal, comparison_size };
//enum AdjacentDirection { direction_up, direction_up_right, direction_right, direction_down_right, direction_down, direction_down_left, direction_left, direction_up_left, direction_size};

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
	shared_ptr<VoxelCollection> voxels;
	VoxelMesh mesh;
	VoxelMesh outlineMesh;
	VoxelMesh zMeshes[64];
	VoxelMesh outlineZMeshes[64];
	void setVoxel(Vector3D v, int color);
	void updateVoxel(Vector3D v, int color);
	bool buildMesh();
	bool meshExists = false;
	void render(int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop);
	void moveLayer(int x, int y, int z, int newX, int newY, int newZ, bool copy);
	json getJSON();
	void setOutline(int o);
	bool fullOutline = false;
	int outlineColor = -1;
private:
	static shared_ptr<VoxelCollection> makeOutlineVoxelCollection(shared_ptr<VoxelCollection> vc, bool full);
	static VoxelMesh buildMeshFromVoxelCollection(shared_ptr<VoxelCollection> vc, bool outline);
	void buildZMeshes(shared_ptr<VoxelCollection> vc, ShaderType type);
	void rebuildZMesh(int x, int y);
};

class Condition {
public:
	Condition() {}
	Condition(json j) {}
	bool compareAll(int result[4]);
	bool compare(int expected, int result);
	int variables[4];
	DynamicParameter parameter;
	DynamicComparison comparison;
};

struct ConditionSet {
	vector<Condition> conditions;
};

class DynamicMesh {
	DynamicMesh() {}
	DynamicMesh(json j);
	json getJSON();
	vector<ConditionSet> conditionSets;
	shared_ptr<SpriteMesh> mesh;
};

class VirtualSprite
{
public:
	VirtualSprite() {}
	VirtualSprite(string chrData, shared_ptr<SpriteMesh> mesh);
	VirtualSprite(json j, map<int, shared_ptr<SpriteMesh>> meshes);
	void render(int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop);
	void renderOAM(shared_ptr<PpuSnapshot> snapshot, int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop);
	void renderNametable(shared_ptr<PpuSnapshot> snapshot, int x, int y, int palette, int nametableX, int nametableY, Crop crop);
	json getJSON();
	int id;
	vector<int> appearancesInRomChr;	// Where does this sprite appear in CHR data? For reference.
	string chrData;
	shared_ptr<SpriteMesh> defaultMesh;	// Default mesh
	vector<DynamicMesh> dynamicMeshes;
	bool hasDynamicMesh = false;
	VoxelMesh previewMeshFront;
private:
	string description = "";
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
	RomInfo romInfo;
	string gameName;
	string romFileName;
	string romDirectory;
	string romFullPath;	// includes rom filename
	string extrasDirectory;
	map<int, shared_ptr<VirtualSprite>> sprites;
	map<string, shared_ptr<VirtualSprite>> spritesByChrData;
	map<int, shared_ptr<SpriteMesh>> meshes;
	shared_ptr<VirtualSprite> getSpriteByChrData(char* data);
	int totalSprites;
	static VoxelMesh getSharedMesh(int zArray[32], ShaderType type);
	static void releaseSharedMesh(string hash, ShaderType type);
	void unload();
	json getJSON();
	static StencilGrouping oamGrouping;
	static StencilGrouping ntGrouping;
private:
	static unordered_map<string, SharedMesh> sharedPaletteMeshes;
	static unordered_map<string, SharedMesh> sharedOutlineMeshes;
	
};

static VoxelMesh buildZMesh(int zArray[32]);
static void buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side);
static void buildSideOutline(vector<OverlayVertex> * vertices, int x, int y, int z, VoxelSide side);
void setVoxelColors(char a, char b, Voxel* row);
bool getBitLeftSide(char byte, int position);
string getPaddedStringFromInt(int i, int length);
int charToInt(char c);

struct SceneSprite {
	shared_ptr<SpriteMesh> mesh;
	int palette;
	int x;
	int y;
	bool mirrorH;
	bool mirrorV;
};