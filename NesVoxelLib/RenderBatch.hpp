#pragma once

#include "N3s3d.hpp"
#include "GameData.hpp"
#include "PpuSnapshot.hpp"
#include "N3sPatternTable.hpp"
#include "Camera.hpp"
#include "N3sMath.hpp"
#include "Scene.hpp"

#define STENCIL_START 2
#define STENCIL_MAX 255
#define STENCIL_HIGHLIGHT 1

class ComputedSprite
{
public:
	inline ComputedSprite() {}
	ComputedSprite(SceneSprite s);
	shared_ptr<VirtualSprite> virtualSprite;
	shared_ptr<SpriteMesh> mesh;
	Vector2D position;
	int palette = 0;
	bool mirrorH = false;
	bool mirrorV = false;
	int stencilGroup = -1;
	bool highlight = false;
};

class ComputedNametable
{
public:
	ComputedNametable();
	ComputedSprite getTile(int x, int y);
	ComputedSprite tiles[64][60];
};

class PaletteDrawCall
{
public:
	inline PaletteDrawCall() {}
	PaletteDrawCall(ComputedSprite s);
	VoxelMesh mesh;
	Vector2F position;
	int palette;
	bool mirrorH;
	bool mirrorV;
	int stencilGroup = -1;
};

struct StencilDrawCall
{
	VoxelMesh mesh;
	Vector2F position;
	bool mirrorH;
	bool mirrorV;;
};

struct OutlineBatch
{
	vector<StencilDrawCall> outlines;
	int palette;
	int color;
	int stencilGroup;
};

class RenderBatch {
public:
	RenderBatch(shared_ptr<GameData> gameData, shared_ptr<PpuSnapshot> snapshot, shared_ptr<VirtualPatternTable> vPatternTable);
	RenderBatch(shared_ptr<GameData> gameData, shared_ptr<Scene> scene);
	void render(shared_ptr<Camera> camera);
	bool highlightsToRender = false;
private:
	void computeSpritesOAM();
	void computeSpritesNametable();
	void processMeshesOAM();
	void processMeshesNT();
	void processStencilsOAM();
	//void processStencilsNT();
	void processNTSameColorAdjacentStencilling(int startingGroupNum);
	void setAdjacentStencilGroups(int x, int y, int colorIndex, int groupNumber);
	void batchDrawCallsOAM();
	void batchDrawCallsNT();
	void batchRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect);
	void batchMeshCropped(ComputedSprite s, Crop crop);
	int incrementStencilNumber();
	int currentStencilNumber = STENCIL_START;
	shared_ptr<GameData> gameData;
	shared_ptr<PpuSnapshot> snapshot;
	shared_ptr<VirtualPatternTable> vPatternTable;
	vector<ComputedSprite> computedSprites;
	ComputedNametable nametable;
	vector<PaletteDrawCall> paletteDrawCalls[8];
	vector<StencilDrawCall> highlights;
	unordered_map<int, shared_ptr<OutlineBatch>> outlineBatches;
	bool renderingOAM = true;
	bool renderingNT = true;
	N3sPalette palette;
};