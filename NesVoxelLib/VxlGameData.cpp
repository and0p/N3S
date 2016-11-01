#include "stdafx.h"
#include "VxlGameData.h"
#include <iostream>
#include <fstream>
#include <algorithm>


CartridgeInfo getCartidgeInfo(char * data)
{
	CartridgeInfo c = CartridgeInfo();
	c.prgSize = *(data + 4);
	c.chrSize = *(data + 5);
	return c;
}

VoxelSprite::VoxelSprite()
{
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
	for (int c = 0; c < 5; c++) {
		offset += 64;
		for (int i = 0; i < 64; i++) {
			voxels[offset + i].color = bitmap.pixels[i];
		}
	}
	meshExists = buildMesh();
	if (meshExists)
		buildZMeshes();
}

void VoxelSprite::render(int x, int y, int palette, bool mirrorH, bool mirrorV)
{
	if (meshExists)
	{
		VxlUtil::selectPalette(palette);
		float posX, posY;
		posX = -1.0f + (pixelSizeW * x);
		posY = 1.0f - (pixelSizeH * y);
		VxlUtil::updateMirroring(mirrorH, mirrorV);
		if(mirrorH)
			posX += (pixelSizeW * 8);
		if (mirrorV)
			posY -= (pixelSizeH * 8);
		VxlUtil::updateWorldMatrix(posX, posY, 0);
		VxlUtil::renderMesh(&mesh);
	}
}

void VoxelSprite::renderPartial(int x, int y, int palette, int xOffset, int width, int yOffset, int height, bool mirrorH, bool mirrorV)
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

json VoxelSprite::getJSON()
{
	json j;

	j["name"] = "Sprite Name";
	j["border"] = 3;
	j["flip z"] = false;
	j["offset"] = 0;
	
	for (int z = 0; z < 32; z++)
	{
		string s = "";
		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				s.append(to_string(getVoxel(x, y, z).color));
			}
			if (y < 7)
				s += "|";
		}
		j["data"].push_back(s);
	}

	return j;
}

Voxel VoxelSprite::getVoxel(int x, int y, int z)
{
	int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
	if (voxelSlot >= 0 && voxelSlot < spriteSize) {
		return voxels[voxelSlot];
	}
	else {
		return { 0 };
	}
}

void VoxelSprite::clear() 
{
	for (int i = 0; i < 2048; i++) 
	{
		voxels[i].color = 0;
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
	mesh = VoxelMesh();
	vector<ColorVertex> vertices;
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
						buildSide(&vertices, x, y, z, color, left);
						sideCount++;
					}
					if (x == spriteWidth - 1 || getVoxel(x + 1, y, z).color == 0) {
						buildSide(&vertices, x, y, z, color, right);
						sideCount++;
					}
					if (y == 0 || getVoxel(x, y - 1, z).color == 0) {
						buildSide(&vertices, x, y, z, color, top);
						sideCount++;
					}
					if (y == spriteHeight - 1 || getVoxel(x, y + 1, z).color == 0) {
						buildSide(&vertices, x, y, z, color, bottom);
						sideCount++;
					}
					if (z == 0 || getVoxel(x - 1, y, z - 1).color == 0) {
						buildSide(&vertices, x, y, z, color, front);
						sideCount++;
					}
					if (z == spriteDepth - 1 || getVoxel(x, y, z + 1).color == 0) {
						buildSide(&vertices, x, y, z, color, back);
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
		meshExists = true;
		return true;
	}
	else
	{
		meshExists = false;
		return false;
	}
}

void VoxelSprite::buildZMeshes()
{
	int i = 0;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			int zArray[32];
			for (int z = 0; z < 32; z++)
			{
				zArray[z] = getVoxel(x, y, z).color;
			}
			zMeshes[i] = VoxelGameData::getSharedMesh(zArray);
			i++;
		}
	}
}

VoxelMesh VoxelSprite::buildZMesh(int zArray[32])
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
			buildSide(&vertices, 0, 0, z, color, left);
			buildSide(&vertices, 0, 0, z, color, right);
			buildSide(&vertices, 0, 0, z, color, top);
			buildSide(&vertices, 0, 0, z, color, bottom);
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

void VoxelSprite::buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side) {
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
	vertices->push_back(v1);
	vertices->push_back(v2);
	vertices->push_back(v4);
	vertices->push_back(v2);
	vertices->push_back(v3);
	vertices->push_back(v4);
}

VoxelGameData::VoxelGameData(char * data, int spriteGroupSize) : spriteGroupSize(spriteGroupSize)
{
	cartridgeInfo = getCartidgeInfo(data);
	// Grab char data, which is offset by 16 bytes due to iNES header
	chrData = data + 16 + (cartridgeInfo.prgSize * 16384);
	totalSprites = cartridgeInfo.chrSize * 512;
	totalSpriteGroups = totalSprites / spriteGroupSize;
	int spriteGroupBytes = spriteGroupSize * 16;
	int hashSize = 32;
	// Make sure hash size isn't somehow larger than sprite group bytes
	if (hashSize > spriteGroupBytes)
		hashSize = spriteGroupBytes;
	bool insertDuplicateSprites = true;	// Assuming for now
	unordered_set<string> hashSet;	// Set of unique hashes to check for duplicates
	unordered_map<string, int> spriteGroupStartingSprites;	// Keep track of which sprites in CHR ROM each sprite group starts at
	// Load sprite group hashes
	for (int i = 0; i < totalSpriteGroups; i++)
	{
		string hash = simpleHash(chrData + (spriteGroupBytes * i), spriteGroupBytes, hashSize);
		if (hashSet.count(hash) < 1)
		{
			hashSet.insert(hash);
			shared_ptr<SpriteGroup> spriteGroup = shared_ptr<SpriteGroup>(new SpriteGroup(hash, spriteGroupSize));
			spriteGroups.push_back(spriteGroup);
			spriteGroupsByHash[hash] = spriteGroup;
			spriteGroupStartingSprites[hash] = i * spriteGroupSize;
		}
	}
	// Load all sprites into unique sprite groups
	hashSet.clear();
	for each (auto kv in spriteGroupStartingSprites)
	{
		
	}
}

VoxelGameData::VoxelGameData(json json)
{
	spriteGroups = vector<shared_ptr<SpriteGroup>>();
	spriteGroupsByHash = unordered_map<string, shared_ptr<SpriteGroup>>();
	sprites = unordered_map<int, VoxelSprite>();
	meshes = unordered_map<int, VoxelMesh>();

	// Load metadata

	//
}

unordered_map<string, SharedMesh> VoxelGameData::sharedMeshes;

void VoxelGameData::createSpritesFromBitmaps()
{
	for (int i = 0; i < totalSprites; i++) {
		sprites[i].buildFromBitmapSprite(bitmaps[i]);
	}
}

void VoxelGameData::buildAllMeshes() {
	for (int i = 0; i < sprites.size(); i++) {
			sprites[i].buildMesh();
	}
}

void VoxelGameData::grabBitmapSprites(const void * gameData)
{
	for (int i = 0; i < totalSprites; i++)
	{
		bitmaps.push_back(BitmapSprite(chrData + (i * 16)));
	}
}

VoxelMesh VoxelGameData::getSharedMesh(int zArray[32])
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
		m.mesh = VoxelSprite::buildZMesh(zArray);
		m.referenceCount++;
		sharedMeshes[hash] = m;
		return m.mesh;
	}
}

void VoxelGameData::releaseSharedMesh(string hash)
{
	// Make sure this actually exists
	if (sharedMeshes.count(hash) > 0)
	{
		sharedMeshes[hash].referenceCount--;
		// Delete this mesh if it is no longer in use
		sharedMeshes[hash].mesh.buffer->Release();
		sharedMeshes.erase(hash);
	}
}

void VoxelGameData::unload()
{
	// Release all full meshes
	for (VoxelSprite sprite : sprites)
	{
		if (sprite.meshExists)
		{
			sprite.mesh.buffer->Release();
		}
	}
	// Release all partial meshes
	for (auto kv : sharedMeshes)
	{
		if(kv.second.mesh.buffer != nullptr)
			kv.second.mesh.buffer->Release();
	}
	sharedMeshes.clear();
}

void VoxelGameData::exportToJSON()
{
	json j;

	// Get Metadata

	// Get data
	for each(VoxelSprite v in sprites)
	{
		j["data"].push_back(v.getJSON());
	}

	ofstream myfile;
	myfile.open("example.json");
	string s = j.dump(4);
	//replace(s.begin(), s.end(), '|', '\n');
	//replace(s.begin(), s.end(), '~', '\t');
	//s = s.replace(s.begin(), s.end(), '|', '\n');
	//s = s.replace(s.begin(), s.end(), '~', '\t');
	myfile << s;
	myfile.close();
		
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

SpriteGroup::SpriteGroup(string hash, int size)
{
	// TODO allocate capacity ahead of time
}
