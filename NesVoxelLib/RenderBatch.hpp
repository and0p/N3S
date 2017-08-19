#pragma once

#include "N3s3d.hpp"
#include "GameData.hpp"
#include "PpuSnapshot.hpp"
#include "N3sPatternTable.hpp"
#include "Camera.hpp"
#include "N3sMath.hpp"

struct ComputedOAMSprite
{
	shared_ptr<VirtualSprite> virtualSprite;
	shared_ptr<SpriteMesh> mesh;
	Vector2D position;
	int palette;
	bool mirrorH;
	bool mirrorV;
	int stencilGroup = -1;
};

struct ComputedNTSprite
{
	shared_ptr<VirtualSprite> virtualSprite;
	shared_ptr<SpriteMesh> mesh;
	int palette;
	int stencilGroup = -1;
};

class ComputedNametable
{
	ComputedNametable(shared_ptr<PpuSnapshot> snapshot);
	ComputedNTSprite tiles[64][60];
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
	vector<ComputedOAMSprite> computedSprites;
	Nametable nametable;
	vector<PaletteDrawCall> paletteDrawCalls[8];
	unordered_map<int, shared_ptr<OutlineBatch>> outlineBatches;
};