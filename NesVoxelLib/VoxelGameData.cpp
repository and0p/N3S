#include "stdafx.h"
#include "VoxelGameData.h"

VoxelSprite::VoxelSprite()
{
	randomizeSprite();
}

void VoxelSprite::setVoxel(int x, int y, int z, int color)
{
	// Calculate which slot in 'flattened' array this pertains to
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	if (voxelSlot >= 0 && voxelSlot < spriteSize) {
		voxels[voxelSlot].color = color;
	}
}


Voxel VoxelSprite::getVoxel(int x, int y, int z)
{
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	if (voxelSlot >= 0 && voxelSlot < spriteSize) {
		return voxels[voxelSlot];
	}
	else {
		return { 0,0 };
	}
}

void VoxelSprite::clearVoxel() 
{
	for (int i = 0; i < 2048; i++) 
	{
		voxels[i].color = 0;
		voxels[i].smooth = 0;
	}
}

void VoxelSprite::randomizeSprite() {
	for (int x = 0; x < spriteWidth; x++)
	{
		for (int y = 0; y < spriteHeight; y++)
		{
			for (int z = 0; z < spriteDepth; z++)
			{
				//int randomBinary = rand() % 2;
				//setVoxel(x, y, z, randomBinary);
				setVoxel(x, y, z, 0);
			}
		}
	}
	for (int i = 0; i < spriteSize; i++)
	{
		voxels[i].smooth = false;
	}
	setVoxel(0, 0, 0, 1);
	setVoxel(0, 1, 0, 1);
	setVoxel(0, 2, 0, 1);
	setVoxel(5, 0, 0, 1);
	setVoxel(6, 1, 0, 1);
	setVoxel(7, 2, 0, 1);
	for (int i = 0; i < spriteDepth; i++) {
		setVoxel(0, 0, i, i % 2);
	}
	for (int i = 0; i < spriteDepth; i++) {
		setVoxel(1, 1, i, i % 2);
	}
}

void VoxelSprite::buildMesh()
{
	//VoxelMesh *mesh;
	mesh = new VoxelMesh;
	std::shared_ptr<std::vector<ColorVertex>> vertices(new std::vector<ColorVertex>);
	//vertices->reserve(256);
	int sideCount = 0;
	for (int z = 0; z < spriteDepth; z++)
	{
		for (int y = 0; y < spriteHeight; y++)
		{
			for (int x = 0; x < spriteWidth; x++)
			{
				// See if the voxel is not empty
				if (getVoxel(x, y, z).color > 0)
				{
					// Check each adjacent voxel/edge and draw a square as needed
					// TODO: Also add > evaluation for semi-transparent voxels in the future
					if (x == 0 || getVoxel(x - 1, y, z).color == 0) {
						buildSide(*vertices, x, y, z, 1, left);
						sideCount++;
					}
					if (x == spriteWidth - 1 || getVoxel(x + 1, y, z).color == 0) {
						buildSide(*vertices, x, y, z, 1, right);
						sideCount++;
					}
					if (y == 0 || getVoxel(x, y - 1, z).color == 0) {
						buildSide(*vertices, x, y, z, 1, top);
						sideCount++;
					}
					if (y == spriteHeight - 1 || getVoxel(x, y + 1, z).color == 0) {
						buildSide(*vertices, x, y, z, 1, bottom);
						sideCount++;
					}
					if (z == 0 || getVoxel(x - 1, y, z - 1).color == 0) {
						buildSide(*vertices, x, y, z, 1, front);
						sideCount++;
					}
					if (z == spriteDepth - 1 || getVoxel(x, y, z + 1).color == 0) {
						buildSide(*vertices, x, y, z, 1, back);
						sideCount++;
					}
				}
			}
		}
	}
	mesh->size = sideCount * 6;
	mesh->type = color;
	mesh->buffer = VoxelUtil::createBufferFromColorVerticesV(*vertices, mesh->size);
}

void VoxelSprite::buildSide(std::vector<ColorVertex> &vertices, int x, int y, int z, int color, VoxelSide side) {
	// Translate voxel-space xyz into model-space based on pixel size in 3d space, phew
	float xf = x * pixelSizeW;
	float yf = y * -pixelSizeH;
	float zf = z * pixelSizeW;
	if (x > 6) {
		int debuuuug = 1;
	}
	// Init 4 vertices
	ColorVertex v1, v2, v3, v4;
	// Set all colors to red for now
	v1.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	v2.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	v3.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	v4.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	// Switch based on side
	switch(side) {
	case left:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		break;
	case right:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf, 1.0f);
		break;
	case top:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		break;
	case bottom:
		v1.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		break;
	case front:
		v1.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf, 1.0f);
		break;
	case back:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		break;
	}
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
}

VoxelGameData::VoxelGameData(int totalSprites, int ppuBankSize): totalSprites(totalSprites), ppuBankSize(ppuBankSize)
{
	// Initialize sprite list
	sprites.reserve(totalSprites);
	// Build all sprites, no meshes yet
	for (int i = 0; i < totalSprites; i++) {
		sprites.push_back(VoxelSprite());
	}
}

void VoxelGameData::buildAllMeshes() {
	for (int i = 0; i < sprites.size(); i++) {
		sprites[i].buildMesh();
	}
}