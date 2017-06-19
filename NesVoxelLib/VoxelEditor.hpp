#pragma once
#include "N3s3d.hpp"
#include "Gamedata.hpp"
#include "N3sMath.hpp"

using namespace std;

class VoxelEditor {
public:
	VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, bool mirrorH, bool mirrorV, OrbitCamera referenceCamera);
	bool update(bool mouseAvailable);
	void render();
	void adjustWorkingPosition(int x, int y, int z);
	void adjustWorkingPositionAnalog(float x, float y, float z);
	void changeLayers(int amount);
	OrbitCamera camera;
	int selectedColor = 1;
	shared_ptr<SpriteMesh> mesh;
	float workingX, workingY, workingZ;
	Vector3D mouseHighlight;
	bool mouseInEditor = false;
private:
	void updateCamera();
	void getMouseHighlight();
	int pixelX;
	int pixelY;
	bool mirrorH;
	bool mirrorV;
	Vector3D selection;
	Vector3D position;
	int xSelect, ySelect, zSelect;
	float editorX, editorY, editorZ;
	int layerNumber;
	ViewingAngle viewingAngle;
};