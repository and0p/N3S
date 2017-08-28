#pragma once

#include "N3s3d.hpp"
#include "GameData.hpp"
#include "PpuSnapshot.hpp"
#include "N3sPatternTable.hpp"
#include "Camera.hpp"
#include "N3sMath.hpp"

struct ComputedSprite
{
	shared_ptr<VirtualSprite> virtualSprite;
	shared_ptr<SpriteMesh> mesh;
	Vector2D position;
	int palette = 0;
	bool mirrorH = false;
	bool mirrorV = false;
	int stencilGroup = -1;
};


struct ComputedNametable
{
	ComputedSprite tiles[64][60];
};

struct PaletteDrawCall
{
	VoxelMesh mesh;
	Vector2F position;
	int palette;
	bool mirrorH;
	bool mirrorV;
	int stencilGroup = -1;
};

struct OutlineDrawCall
{
	VoxelMesh mesh;
	Vector2F position;
	bool mirrorH;
	bool mirrorV;;
};

struct OutlineBatch
{
	vector<OutlineDrawCall> outlines;
	int palette;
	int color;
	int stencilGroup;
};

class RenderBatch {
public:
	RenderBatch(shared_ptr<GameData> gameData, shared_ptr<PpuSnapshot> snapshot, shared_ptr<VirtualPatternTable> vPatternTable);
	void render(shared_ptr<Camera> camera);
private:
	void computeSpritesOAM();
	void computeSpritesNametable();
	void processMeshesOAM();
	void processMeshesNT();
	void processStencilGroups();
	void batchDrawCallsOAM();
	void batchDrawCallsNT();
	void batchRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect);
	void batchMeshCropped(ComputedSprite s, Crop crop);
	int currentStencilNumber = 1;
	shared_ptr<GameData> gameData;
	shared_ptr<PpuSnapshot> snapshot;
	shared_ptr<VirtualPatternTable> vPatternTable;
	vector<ComputedSprite> computedSprites;
	ComputedNametable nametable;
	vector<PaletteDrawCall> paletteDrawCalls[8];
	unordered_map<int, shared_ptr<OutlineBatch>> outlineBatches;
};