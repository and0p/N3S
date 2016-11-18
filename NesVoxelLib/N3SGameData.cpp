#include "stdafx.h"
#include "N3SGameData.hpp"


RomInfo getRomInfo(char * data)
{
	RomInfo r;
	r.prgSize = *(data + 4);
	r.chrSize = *(data + 5);
	return r;
}

// Generate SpriteMesh from CHR data in a ROM
SpriteMesh::SpriteMesh(char* spriteData)
{
	voxels.reset(new VoxelCollection());
	voxels->clear();
	int voxelStart = spriteSize / 2;
	for (int i = 0; i < 8; i++)
	{
		setVoxelColors(*(spriteData + (i * 2)), *(spriteData + (i * 2) + 1), &voxels->voxels[voxelStart + (i * 8)]);
	}
	meshExists = buildMesh();
}

SpriteMesh::SpriteMesh(json data, bool edit)
{
	voxels.reset(new VoxelCollection());
	// Load voxels
	
	// Build mesh
	meshExists = buildMesh();
	// If we're not in editor mode, don't keep the voxel array loaded
	// voxels.release();
}

void SpriteMesh::setVoxel(int x, int y, int z, int color)
{
	// Calculate which slot in 'flattened' array this pertains to
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	if (voxelSlot >= 0 && voxelSlot < spriteSize) 
	{
		voxels->voxels[voxelSlot].color = color;
	}
}

void SpriteMesh::buildZMeshes()
{
	int i = 0;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			int zArray[32];
			for (int z = 0; z < 32; z++)
			{
				zArray[z] = voxels->getVoxel(x, y, z).color;
			}
			zMeshes[i] = GameData::getSharedMesh(zArray);
			i++;
		}
	}
}

VoxelMesh buildZMesh(int zArray[32])
{
	VoxelMesh mesh = VoxelMesh();
	vector<ColorVertex> vertices;
	int sideCount = 0;
	for (int z = 0; z < spriteDepth; z++)
	{
		int color = zArray[z];
		// See if the voxel is not empty
		if (color > 0)
		{
			// Draw left, right, top, and bottom edges, which should be visible on Z-Meshes no matter what
			buildSide(&vertices, 0, 0, z, color, VoxelSide::left);
			buildSide(&vertices, 0, 0, z, color, VoxelSide::right);
			buildSide(&vertices, 0, 0, z, color, VoxelSide::top);
			buildSide(&vertices, 0, 0, z, color, VoxelSide::bottom);
			sideCount += 4;
			// See if front and/or back are exposed, and add sides if so
			if (z == 0 || zArray[z - 1] == 0)
			{
				buildSide(&vertices, 0, 0, z, color, front);
				sideCount++;
			}
			if (z == 31 || zArray[z + 1] == 0)
			{
				buildSide(&vertices, 0, 0, z, color, back);
				sideCount++;
			}
			// TODO: Also add > evaluation for semi-transparent voxels in the future
		}
	}
	mesh.size = sideCount * 6;
	mesh.type = color;
	mesh.buffer = VxlUtil::createBufferFromColorVerticesV(&vertices, mesh.size);
	return mesh;
}

void buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side) {
	// Translate voxel-space xyz into model-space based on pixel size in 3d space, phew
	float xf = x * pixelSizeW;
	float yf = y * -pixelSizeH;
	float zf = z * pixelSizeW;
	// Init 4 vertices
	ColorVertex v1, v2, v3, v4;
	// Set all colors to greyscale
	if (color == 1)
	{
		v4.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		v1.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		v2.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
		v3.Col = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (color == 2)
	{
		v1.Col = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		v2.Col = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		v3.Col = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
		v4.Col = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (color == 3)
	{
		v1.Col = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
		v2.Col = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
		v3.Col = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
		v4.Col = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	}

	// Switch based on side
	switch (side) {
	case VoxelSide::left:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		break;
	case VoxelSide::right:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf, 1.0f);
		break;
	case VoxelSide::top:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		break;
	case VoxelSide::bottom:
		v1.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		break;
	case VoxelSide::front:
		v1.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf, 1.0f);
		break;
	case VoxelSide::back:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeH, zf + pixelSizeW, 1.0f);
		break;
	}
	vertices->push_back(v1);
	vertices->push_back(v2);
	vertices->push_back(v4);
	vertices->push_back(v2);
	vertices->push_back(v3);
	vertices->push_back(v4);
}

GameData::GameData(char * data)
{
	// Get ROM metadata
	romInfo = getRomInfo(data);
	// Grab char data, which is offset by 16 bytes due to iNES header
	char* chrData = data + 16 + (romInfo.prgSize * 16384);
	totalSprites = romInfo.chrSize * 512;
	bool ignoreDuplicateSprites = true; // Temporary, this will eventually get specified by user
	// Create maps to ignore duplicate sprites
	unordered_set<string> previousSprites;
	int spritesIgnored = 0;
	// Loop through and create all static sprites
	for (int i = 0; i < totalSprites; i++)
	{
		// Build a string that contains the 16 bytes of current sprite
		char* current = chrData + (i * 16);
		string spriteData = "";
		for (int i = 0; i < 16; i++)
		{
			spriteData += *(current + i);
		}
		// Check for duplicate
		if (previousSprites.count(spriteData) < 1)
		{
			int currentMesh = i - spritesIgnored;
			// Add the data of the sprite we're going to generate to the map of duplicates
			previousSprites.insert(spriteData);
			// Build mesh and add to list
			meshes[currentMesh] = shared_ptr<SpriteMesh>(new SpriteMesh(current));
			// Build the static sprite
			shared_ptr<StaticSprite> staticSprite = shared_ptr<StaticSprite>(new StaticSprite(spriteData, meshes[currentMesh]));
			sprites[i] = staticSprite;
		}
		else
		{
			spritesIgnored++;
		}
	}
}



GameData::GameData(json data)
{
	// Load metadata
	// Load meshes
	//map<int, VoxelMesh> meshes;
	//for (int i = 0; i < data["meshes"].size(); i++)
	//{
	//	// SpriteMesh mesh = SpriteMesh(data["meshes"][i]["voxels"]);
	//	//VoxelMesh mesh;
	//}
}

shared_ptr<VirtualSprite> GameData::getSpriteByChrData(char * data)
{
	// Get string of CHR data
	string s = "";
	for (int i = 0; i < 16; i++)
	{
		s += *(data + i);
	}
	// Return from sprites if it exists, otherwise return sprite 0 :(
	if (spritesByChrData.count(s) > 0)
		return spritesByChrData[s];
	else
		return sprites[0];
}

VoxelMesh GameData::getSharedMesh(int zArray[32])
{
	// Create simple string hash from the voxels
	string hash = "";
	char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	for (int z = 0; z < 31; z++)
	{
		hash += digits[zArray[z]];
		hash += '.';
	}
	hash += digits[zArray[31]];
	// Return mesh if it exists, otherwise create
	if (sharedMeshes.count(hash) > 0)
	{
		// Increase number of references
		sharedMeshes[hash].referenceCount++;
		// Return reference
		return sharedMeshes[hash].mesh;
	}
	else
	{
		SharedMesh m = SharedMesh();
		m.mesh = buildZMesh(zArray);
		m.referenceCount++;
		sharedMeshes[hash] = m;
		return m.mesh;
	}
}

void GameData::releaseSharedMesh(string hash)
{
	// Make sure this actually exists
	if (sharedMeshes.count(hash) > 0)
	{
		sharedMeshes[hash].referenceCount--;
		// Delete this mesh if it is no longer in use
		if (sharedMeshes[hash].referenceCount <= 0)
		{
			sharedMeshes[hash].mesh.buffer->Release();
			sharedMeshes.erase(hash);
		}
	}
}

bool SpriteMesh::buildMesh()
{
	vector<ColorVertex> vertices;
	int sideCount = 0;
	for (int z = 0; z < spriteDepth; z++)
	{
		for (int y = 0; y < spriteHeight; y++)
		{
			for (int x = 0; x < spriteWidth; x++)
			{
				int color = voxels->getVoxel(x, y, z).color;
				// See if the voxel is not empty
				if (color > 0)
				{
					// Check each adjacent voxel/edge and draw a square as needed
					// TODO: Also add > evaluation for semi-transparent voxels in the future?
					if (x == 0 || voxels->getVoxel(x - 1, y, z).color == 0) {
						buildSide(&vertices, x, y, z, color, VoxelSide::left);
						sideCount++;
					}
					if (x == spriteWidth - 1 || voxels->getVoxel(x + 1, y, z).color == 0) {
						buildSide(&vertices, x, y, z, color, VoxelSide::right);
						sideCount++;
					}
					if (y == 0 || voxels->getVoxel(x, y - 1, z).color == 0) {
						buildSide(&vertices, x, y, z, color, VoxelSide::top);
						sideCount++;
					}
					if (y == spriteHeight - 1 || voxels->getVoxel(x, y + 1, z).color == 0) {
						buildSide(&vertices, x, y, z, color, VoxelSide::bottom);
						sideCount++;
					}
					if (z == 0 || voxels->getVoxel(x - 1, y, z - 1).color == 0) {
						buildSide(&vertices, x, y, z, color, VoxelSide::front);
						sideCount++;
					}
					if (z == spriteDepth - 1 || voxels->getVoxel(x, y, z + 1).color == 0) {
						buildSide(&vertices, x, y, z, color, VoxelSide::back);
						sideCount++;
					}
				}
			}
		}
	}
	mesh.size = sideCount * 6;
	mesh.type = color;
	// Return true if there is an actual mesh to render
	if (vertices.size() > 0)
	{
		mesh.buffer = VxlUtil::createBufferFromColorVerticesV(&vertices, mesh.size);
		buildZMeshes();
		return true;
	}
	else
	{
		return false;
	}
}

VoxelCollection::VoxelCollection()
{
	clear();
}

VoxelCollection::VoxelCollection(char * sprite)
{
	clear();
	int spriteGraphic[64];
	int offset = 16;
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			// setVoxel(x, y,)
		}
	}
}

Voxel VoxelCollection::getVoxel(int x, int y, int z)
{
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	return voxels[voxelSlot];
}

void VoxelCollection::setVoxel(int x, int y, int z, Voxel v)
{
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	voxels[voxelSlot] = v;
}

void VoxelCollection::clear()
{
	for (int i = 0; i < spriteSize; i++)
	{
		voxels[i].color = 0;
	}
}

// Set the color of 8 consecutive voxels using CHR sprite data
void setVoxelColors(char a, char b, Voxel* row)
{
	for (int i = 0; i < 8; i++)
	{
		int color = 0;
		bool bit1 = getBitLeftSide(a, i);
		bool bit2 = getBitLeftSide(b, i);
		if (bit1)
			color += 1;
		if (bit2)
			color += 2;
		(row + i)->color = color;
	}
}

bool getBit(char byte, int position)
{
	return (byte >> position) & 0x1;
}

bool getBitLeftSide(char byte, int position)
{
	return (byte << position) & 0x80;
}

VirtualSprite::VirtualSprite()
{
}

VirtualSprite::VirtualSprite(string data) : data (data)
{
}


StaticSprite::StaticSprite(string data, shared_ptr<SpriteMesh> mesh)
	: VirtualSprite(data), mesh(mesh)
{

}

void SpriteMesh::render(int x, int y, int palette, bool mirrorH, bool mirrorV)
{
	if (meshExists)
	{
		VxlUtil::selectPalette(palette);
		float posX, posY;
		posX = -1.0f + (pixelSizeW * x);
		posY = 1.0f - (pixelSizeH * y);
		VxlUtil::updateMirroring(mirrorH, mirrorV);
		if (mirrorH)
			posX += (pixelSizeW * 8);
		if (mirrorV)
			posY -= (pixelSizeH * 8);
		VxlUtil::updateWorldMatrix(posX, posY, 0);
		VxlUtil::renderMesh(&mesh);
	}
}

void SpriteMesh::renderPartial(int x, int y, int palette, int xOffset, int width, int yOffset, int height, bool mirrorH, bool mirrorV)
{
	if (meshExists)
	{
		VxlUtil::selectPalette(palette);
		for (int posY = 0; posY < height; posY++)
		{
			for (int posX = 0; posX < width; posX++)
			{
				// Grab different zMeshes based on mirroring
				int meshX, meshY;
				if (mirrorH)
					meshX = 7 - (posX + xOffset);
				else
					meshX = posX + xOffset;
				if (mirrorV)
					meshY = 7 - (posY + yOffset);
				else
					meshY = posY + yOffset;
				if (zMeshes[(meshY * 8) + meshX].buffer != nullptr)
				{
					VxlUtil::updateWorldMatrix(-1.0f + ((x + posX) * pixelSizeW), 1.0f - ((y + posY) * pixelSizeH), 0);
					VxlUtil::renderMesh(&zMeshes[(meshY * 8) + meshX]);
				}
			}
		}
	}

}
