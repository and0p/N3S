#pragma once
#include "stdafx.h"
#include "N3s3d.hpp"

using namespace std;

class VoxelEditor {
public:
	VoxelEditor(shared_ptr<VoxelMesh> mesh, int pixelX, int pixelY) : mesh(mesh), pixelX(pixelX), pixelY(pixelY) {}
	bool update(bool mouseAvailable);
	void render();
	shared_ptr<OrbitCamera> camera;
private:
	shared_ptr<VoxelMesh> mesh;
	int pixelX;
	int pixelY;
};