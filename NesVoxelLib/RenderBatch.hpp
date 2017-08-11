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
};

struct PaletteDrawCall
{
	VoxelMesh mesh;
	float x, y;
	int palette;
};

struct OutlineDrawCall
{
	VoxelMesh mesh;
	float x, y;
};

struct OutlineBatch
{
	vector<OutlineDrawCall> outlines;
	int palette;
	int color;
};

class RenderBatch {
public:
	RenderBatch(shared_ptr<GameData> gameData, shared_ptr<PpuSnapshot> snapshot, shared_ptr<VirtualPatternTable> vPatternTable);
	void processOAM();
	void processNametable();
	void processMeshes();
	void render(shared_ptr<Camera> camera);
private:
	shared_ptr<GameData> gameData;
	shared_ptr<PpuSnapshot> snapshot;
	shared_ptr<VirtualPatternTable> vPatternTable;
	vector<ComputedSprite> computedSprites;
	vector<OutlineBatch> outlineBatches;
};