#pragma once
#include "N3s3d.hpp"
#include "Gamedata.hpp"
#include "N3sMath.hpp"

using namespace std;

class VoxelEditor {
public:
	VoxelEditor(shared_ptr<SpriteMesh> mesh, SceneSprite *ss, shared_ptr<OrbitCamera> referenceCamera);
	bool update(bool mouseAvailable);
	void render();
	void renderGrid(Vector3D v, ViewingAngle vA, Color4F color, bool depthEnabled);
	void adjustWorkingPosition(int x, int y, int z);
	void adjustWorkingPositionAnalog(float x, float y, float z);
	void changeLayers(int amount);
	shared_ptr<OrbitCamera> camera;
	int selectedColor = 1;
	shared_ptr<SpriteMesh> mesh;
	float workingX, workingY, workingZ;
	Vector3D mouseHighlight;
	bool mouseInEditor = false;
	void changeSprite(SceneSprite *newSprite);
private:
	Crop crop;
	void updateCamera();
	void updateCroping();
	void getMouseHighlight();
	void setMirroring();
	SceneSprite *ss;
	Vector3D selection;
	Vector3D mirroredSelection;
	Vector3D position;
	Vector3D mirrorPoint;
	Vector3D mirroredMouseHighlight;
	MirrorDirection mirrorDirection;
	MirrorStyle mirrorStyle;
	int xSelect, ySelect, zSelect;
	float editorX, editorY, editorZ;
	int layerNumber;
	ViewingAngle viewingAngle;
};