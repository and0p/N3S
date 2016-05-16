#include "stdafx.h"
#include "VoxelGameData.h"

VoxelSprite::VoxelSprite()
{
	//randomizeSprite();
	meshExists = false;
	clear();
}

void VoxelSprite::setVoxel(int x, int y, int z, int color)
{
	// Calculate which slot in 'flattened' array this pertains to
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	if (voxelSlot >= 0 && voxelSlot < spriteSize) {
		voxels[voxelSlot].color = color;
	}
}

void VoxelSprite::buildFromBitmapSprite(BitmapSprite bitmap)
{
	// Offset to middle of z-index
	int offset = 64 * 8;
	// Insert all bitmap colors in that z-column
	for (int c = 0; c < 6; c++) {
		offset += 64;
		for (int i = 0; i < 64; i++) {
			voxels[offset + i].color = bitmap.pixels[i];
		}
	}
	meshExists = buildMesh();
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

void VoxelSprite::clear() 
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

bool VoxelSprite::buildMesh()
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
				int color = getVoxel(x, y, z).color;
				// See if the voxel is not empty
				if (color > 0)
				{
					// Check each adjacent voxel/edge and draw a square as needed
					// TODO: Also add > evaluation for semi-transparent voxels in the future
					if (x == 0 || getVoxel(x - 1, y, z).color == 0) {
						buildSide(*vertices, x, y, z, color, left);
						sideCount++;
					}
					if (x == spriteWidth - 1 || getVoxel(x + 1, y, z).color == 0) {
						buildSide(*vertices, x, y, z, color, right);
						sideCount++;
					}
					if (y == 0 || getVoxel(x, y - 1, z).color == 0) {
						buildSide(*vertices, x, y, z, color, top);
						sideCount++;
					}
					if (y == spriteHeight - 1 || getVoxel(x, y + 1, z).color == 0) {
						buildSide(*vertices, x, y, z, color, bottom);
						sideCount++;
					}
					if (z == 0 || getVoxel(x - 1, y, z - 1).color == 0) {
						buildSide(*vertices, x, y, z, color, front);
						sideCount++;
					}
					if (z == spriteDepth - 1 || getVoxel(x, y, z + 1).color == 0) {
						buildSide(*vertices, x, y, z, color, back);
						sideCount++;
					}
				}
			}
		}
	}
	mesh->size = sideCount * 6;
	mesh->type = color;
	// Return true if there is an actual mesh to render
	if (vertices->size() > 0) {
		mesh->buffer = VoxelUtil::createBufferFromColorVerticesV(*vertices, mesh->size);
		meshExists = true;
		return true;
	}
	else
	{
		meshExists = false;
		return false;
	}
}

void VoxelSprite::buildSide(std::vector<ColorVertex> &vertices, int x, int y, int z, int color, VoxelSide side) {
	// Translate voxel-space xyz into model-space based on pixel size in 3d space, phew
	float xf = x * pixelSizeW;
	float yf = y * -pixelSizeH;
	float zf = z * pixelSizeW;
	// Init 4 vertices
	ColorVertex v1, v2, v3, v4;
	// Set all colors to red for now
	if (color == 1)
	{
		v4.Col = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		v1.Col = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		v2.Col = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		v3.Col = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	}
	else if (color == 2)
	{
		v1.Col = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
		v2.Col = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
		v3.Col = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
		v4.Col = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	}
	else if (color == 3)
	{
		v1.Col = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
		v2.Col = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
		v3.Col = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
		v4.Col = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	}
	
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
	bitmaps.reserve(totalSprites);
	// Build all sprites, no meshes yet
	for (int i = 0; i < totalSprites; i++) {
		sprites.push_back(VoxelSprite());
		//bitmaps.push_back(BitmapSprite());
	}
}

void VoxelGameData::createSpritesFromBitmaps()
{
	for (int i = 0; i < totalSprites; i++) {
		sprites[i].buildFromBitmapSprite(bitmaps[i]);
	}
}

void VoxelGameData::buildAllMeshes() {
	for (int i = 0; i < sprites.size(); i++) {
		if (sprites.size() > 0) {
			sprites[i].buildMesh();
		}
	}
}

void VoxelGameData::grabBitmapSprites(const void * gameData, int offsetBytes)
{
	char *charData = ((char*)gameData);
	for (int i = 0; i < totalSprites; i++)
	{
		bitmaps.push_back(BitmapSprite(charData + offsetBytes + (i * 16)));
	}
}

// Create blank sprite
BitmapSprite::BitmapSprite()
{
	for (int i = 0; i < 64; i++)
	{
		pixels[i] = 0;
	}
}

// Create sprite from data
BitmapSprite::BitmapSprite(char * data)
{
	// Sprite is 16 bytes, 2 per row
	// Iterate and add all rows
	for (int i = 0; i < 8; i++)
	{
		int offset = i;
		addPixels(*(data + offset), *(data + offset + 8), i);
	}
}

BitmapSprite::~BitmapSprite()
{
}

// Add pixels to specified row using both bytes of sprite row
void BitmapSprite::addPixels(char a, char b, int row)
{
	int rowOffset = row * 8;
	for (int i = 0; i < 8; i++)
	{
		int color = 0;
		bool bit1 = getBitLeftSide(a, i);
		bool bit2 = getBitLeftSide(b, i);
		if (bit1)
			color += 1;
		if (bit2)
			color += 2;
		pixels[rowOffset + i] = color;
	}
}

bool BitmapSprite::getBit(char byte, int position)
{
	return (byte >> position) & 0x1;
}

bool BitmapSprite::getBitLeftSide(char byte, int position)
{
	return (byte << position) & 0x80;
}