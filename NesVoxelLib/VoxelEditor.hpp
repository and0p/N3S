#pragma once
#include "stdafx.h"
#include "N3s3d.hpp"
#include "Gamedata.hpp"

using namespace std;

class VoxelEditor {
public:
	VoxelEditor(shared_ptr<SpriteMesh> mesh, int pixelX, int pixelY, OrbitCamera referenceCamera);
	bool update(bool mouseAvailable);
	void render();
	OrbitCamera camera;
private:
	shared_ptr<SpriteMesh> mesh;
	int pixelX;
	int pixelY;
};