#pragma once

#include "N3s3d.hpp"
#include "GameData.hpp"
#include "PpuSnapshot.hpp"
#include "N3sPatternTable.hpp"
#include "Camera.hpp"

struct ComputedSprite
{
	shared_ptr<VirtualSprite> virtualSprite;
	shared_ptr<SpriteMesh> mesh;
	int x;
	int y;
	int palette;
	bool mirrorH;
	bool mirrorV;
	int stencilGroup = -1;
};

struct PaletteDrawCall
{
	VoxelMesh mesh;
	Vector2F position;
	int palette;
	int stencilGroup = -1;
};

struct OutlineDrawCall
{
	VoxelMesh mesh;
	Vector2F position;
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
	void computeSpritesOAM();
	void computeSpritesNametable();
	void processMeshesOAM();
	void processStencilGroups();
	void batchDrawCalls();
	void render(shared_ptr<Camera> camera);
private:
	int currentStencilNumber = 1;
	shared_ptr<GameData> gameData;
	shared_ptr<PpuSnapshot> snapshot;
	shared_ptr<VirtualPatternTable> vPatternTable;
	vector<ComputedSprite> computedSprites;
	vector<OutlineBatch> outlineBatches;
};