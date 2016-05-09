#include "stdafx.h"
#include "VoxelGameData.h"

void VoxelSprite::addVoxel(int x, int y, int z)
{
}

void VoxelSprite::deleteVoxel(int x, int y, int z)
{
}

void VoxelSprite::buildMesh()
{
	VoxelMesh *mesh;
	mesh = new VoxelMesh;
	std::vector<ColorVertex> vertices;
	vertices.reserve(256);
	for (int x = 0; x < spriteWidth; x++)
	{
		for (int y = 0; y < spriteHeight; y++)
		{
			for (int z = 0; z < spriteDepth; z++)
			{
				// See if the voxel is not empty
				if (getVoxel(x, y, z) > 0)
				{
					// Check each adjacent voxel/edge and draw a square as needed
					// TODO: Also add > evaluation for semi-transparent voxels in the future
					if (x == 0 || getVoxel(x - 1, y, z) > 0) {
						buildSide(vertices, x, y, z, 1, left);
					}
					if (x == spriteWidth - 1 || getVoxel(x + 1, y, z) > 0) {
						buildSide(vertices, x, y, z, 1, right);
					}
					if (y == 0 || getVoxel(x, y - 1, z) > 0) {
						buildSide(vertices, x, y, z, 1, top);
					}
					if (y == spriteHeight - 1 || getVoxel(x, y + 1, z) > 0) {
						buildSide(vertices, x, y, z, 1, bottom);
					}
					if (z == 0 || getVoxel(x - 1, y, z - 1) > 0) {
						buildSide(vertices, x, y, z, 1, front);
					}
					if (z == spriteDepth - 1 || getVoxel(x, y, z + 1) > 0) {
						buildSide(vertices, x, y, z, 1, back);
					}
				}
			}
		}
	}
	mesh->buffer = VoxelUtil::createBufferFromColorVertices(vertices, vertices.size);
}

void VoxelSprite::buildSide(std::vector<ColorVertex>& vertices, int x, int y, int z, int color, VoxelSide side) {
	// Translate voxel-space xyz into model-space based on pixel size in 3d space, phew
	x *= pixelSizeW;
	y *= pixelSizeH;
	z *= pixelSizeW;
	// Init 4 vertices
	ColorVertex v1, v2, v3, v4;
	v1.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	v2.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	v3.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	v4.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	// Switch based on side
	switch(side) {
	case left:
		v1.Pos = XMFLOAT4(x, y, z - pixelSizeW, 0.0f);
		v2.Pos = XMFLOAT4(x, y, z, 0.0f);
		v3.Pos = XMFLOAT4(x, y - pixelSizeH, z, 0.0f);
		v4.Pos = XMFLOAT4(x, y - pixelSizeH, z - pixelSizeW, 0.0f);
		break;
	case right:
		v1.Pos = XMFLOAT4(x, y, z, 0.0f);
		v2.Pos = XMFLOAT4(x, y, z - pixelSizeW, 0.0f);
		v3.Pos = XMFLOAT4(x + pixelSizeW, y - pixelSizeH, z - pixelSizeW, 0.0f);
		v4.Pos = XMFLOAT4(x + pixelSizeW, y - pixelSizeH, z, 0.0f);
		break;
	case top:
		v1.Pos = XMFLOAT4(x, y, z - pixelSizeW, 0.0f);
		v2.Pos = XMFLOAT4(x + pixelSizeW, y, z - pixelSizeW, 0.0f);
		v3.Pos = XMFLOAT4(x + pixelSizeW, y, z, 0.0f);
		v4.Pos = XMFLOAT4(x, y, z, 0.0f);
		break;
	case bottom:
		v1.Pos = XMFLOAT4(x, y - pixelSizeH, z, 0.0f);
		v2.Pos = XMFLOAT4(x + pixelSizeW, y - pixelSizeH, z, 0.0f);
		v3.Pos = XMFLOAT4(x + pixelSizeW, y - pixelSizeH, z - pixelSizeW, 0.0f);
		v4.Pos = XMFLOAT4(x, y - pixelSizeH, z - pixelSizeW, 0.0f);
		break;
	}
}

int VoxelSprite::getVoxel(int x, int y, int z)
{
	return 0;
}
