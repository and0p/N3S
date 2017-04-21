#pragma once
#include "N3s3d.hpp"
#include "Gamedata.hpp"

using namespace std;

class VoxelEditor {
public:
	VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, OrbitCamera referenceCamera);
	bool update(bool mouseAvailable);
	void render();
	void adjustWorkingPosition(int x, int y, int z);
	void adjustWorkingPositionAnalog(float x, float y, float z);
	void changeLayers(int amount);
	OrbitCamera camera;
private:
	void updateCamera();
	shared_ptr<SpriteMesh> mesh;
	int pixelX;
	int pixelY;
	int xSelect, ySelect, zSelect;
	float workingX, workingY, workingZ;
	float editorX, editorY, editorZ;
	ViewingAngle viewingAngle;
};