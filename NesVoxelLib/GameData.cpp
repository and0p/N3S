#include "stdafx.h"
#include "GameData.hpp"
#include "N3sConsole.hpp"

char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
unordered_map<string, SharedMesh> GameData::sharedPaletteMeshes;
unordered_map<string, SharedMesh> GameData::sharedOutlineMeshes;

StencilGrouping GameData::oamGrouping = continous_samecolor;
StencilGrouping GameData::ntGrouping = adjacent_samecolor;

unordered_map<string, DynamicComparison> comparisonNameMap;
unordered_map<string, DynamicParameter> parameterNameMap;
string comparisonNames[comparison_size] = { "equal", "not_equal", "greater", "less", "greater_or_equal", "less_or_equal" };
string parameterNames[parameter_size] = { "color_id", "all_color_ids", "palette_num", "screen_coordinates", "absolute_nametable", "relative_nametable", "is_oam" };

// Normals for different sides
// left, right, top, bottom, front, back
XMFLOAT4 sideNormals[6] =
{
	{ -1.0f, 0.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, -1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f }
};

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
		setVoxelColors(*(spriteData + i), *(spriteData + i + 8), &voxels->voxels[voxelStart + (i * 8)]);
	}
	meshExists = buildMesh();
}

SpriteMesh::SpriteMesh(json j)
{
	// Grab ID
	id = stoi(j["id"].get<string>());
	// Load voxels
	voxels.reset(new VoxelCollection(j["voxels"]));
	// Build mesh
	meshExists = buildMesh();
	// If we're not in editor mode, don't keep the voxel array loaded
	// voxels.release();
}

void SpriteMesh::setVoxel(Vector3D v, int color)
{
	// Calculate which slot in 'flattened' array this pertains to
	int voxelSlot = v.x + (v.y * spriteWidth) + (v.z * (spriteHeight * spriteWidth));
	if (voxelSlot >= 0 && voxelSlot < spriteSize) 
	{
		voxels->voxels[voxelSlot].color = color;
	}
}

void SpriteMesh::updateVoxel(Vector3D v, int color)
{
	setVoxel(v, color);
	buildMesh();
	rebuildZMesh(v.x, v.y);
}

shared_ptr<VoxelCollection> SpriteMesh::makeOutlineVoxelCollection(shared_ptr<VoxelCollection> vc, bool full)
{
	shared_ptr<VoxelCollection> outlineVC = make_shared<VoxelCollection>();
	// Create all outline voxels
	for (int z = 0; z < spriteDepth; z++)
		for (int y = 0; y < spriteHeight; y++)
			for (int x = 0; x < spriteWidth; x++)
				if (vc->getVoxel(x, y, z).color > 0) // TODO: do I want to grab background color voxels? Probably
				{
					// See if we're doing a full or partial outline
					if (full)
					{
						// Add all 9 possible outline voxels
						for (int newX = -1; newX < 2; newX++)
							for (int newY = -1; newY < 2; newY++)
								for (int newZ = -1; newZ < 2; newZ++)
									outlineVC->setVoxel(x + newX, y + newY, z + newZ, { 1 });
					}
					else
					{
						// Create outline on adjacent, not diagonal, spots
						outlineVC->setVoxel(x, y, z, { 1 });
						outlineVC->setVoxel(x + 1, y, z, { 1 });
						outlineVC->setVoxel(x - 1, y, z, { 1 });
						outlineVC->setVoxel(x, y + 1, z, { 1 });
						outlineVC->setVoxel(x, y - 1, z, { 1 });
						outlineVC->setVoxel(x, y, z + 1, { 1 });
						outlineVC->setVoxel(x, y, z - 1, { 1 });
					}
				}
	return outlineVC;
}

VoxelMesh SpriteMesh::buildMeshFromVoxelCollection(shared_ptr<VoxelCollection> vc, bool outline)
{
	VoxelMesh m;
	vector<ColorVertex> vertices;
	vector<OverlayVertex> outlineVertices;	// Probably not needed
	int sideCount = 0;
	if(outline)
		for (int z = 0; z < spriteDepth; z++)
		{
			for (int y = 0; y < spriteHeight; y++)
			{
				for (int x = 0; x < spriteWidth; x++)
				{
					int color = vc->getVoxel(x, y, z).color;
					// See if the voxel is not empty
					if (color > 0)
					{
						// Check each adjacent voxel/edge and draw a square as needed
						// TODO: Also add > evaluation for semi-transparent voxels in the future?
						if (x == 0 || vc->getVoxel(x - 1, y, z).color == 0) {
							buildSideOutline(&outlineVertices, x, y, z, VoxelSide::left);
							sideCount++;
						}
						if (x == spriteWidth - 1 || vc->getVoxel(x + 1, y, z).color == 0) {
							buildSideOutline(&outlineVertices, x, y, z, VoxelSide::right);
							sideCount++;
						}
						if (y == 0 || vc->getVoxel(x, y - 1, z).color == 0) {
							buildSideOutline(&outlineVertices, x, y, z, VoxelSide::top);
							sideCount++;
						}
						if (y == spriteHeight - 1 || vc->getVoxel(x, y + 1, z).color == 0) {
							buildSideOutline(&outlineVertices, x, y, z, VoxelSide::bottom);
							sideCount++;
						}
						if (z == 0 || vc->getVoxel(x, y, z - 1).color == 0) {
							buildSideOutline(&outlineVertices, x, y, z, VoxelSide::front);
							sideCount++;
						}
						if (z == spriteDepth - 1 || vc->getVoxel(x, y, z + 1).color == 0) {
							buildSideOutline(&outlineVertices, x, y, z, VoxelSide::back);
							sideCount++;
						}
					}
				}
			}
		}
	else // Normal color mesh
	{
		for (int z = 0; z < spriteDepth; z++)
		{
			for (int y = 0; y < spriteHeight; y++)
			{
				for (int x = 0; x < spriteWidth; x++)
				{
					int color = vc->getVoxel(x, y, z).color;
					// See if the voxel is not empty
					if (color > 0)
					{
						// Check each adjacent voxel/edge and draw a square as needed
						// TODO: Also add > evaluation for semi-transparent voxels in the future?
						if (x == 0 || vc->getVoxel(x - 1, y, z).color == 0) {
							buildSide(&vertices, x, y, z, color, VoxelSide::left);
							sideCount++;
						}
						if (x == spriteWidth - 1 || vc->getVoxel(x + 1, y, z).color == 0) {
							buildSide(&vertices, x, y, z, color, VoxelSide::right);
							sideCount++;
						}
						if (y == 0 || vc->getVoxel(x, y - 1, z).color == 0) {
							buildSide(&vertices, x, y, z, color, VoxelSide::top);
							sideCount++;
						}
						if (y == spriteHeight - 1 || vc->getVoxel(x, y + 1, z).color == 0) {
							buildSide(&vertices, x, y, z, color, VoxelSide::bottom);
							sideCount++;
						}
						if (z == 0 || vc->getVoxel(x, y, z - 1).color == 0) {
							buildSide(&vertices, x, y, z, color, VoxelSide::front);
							sideCount++;
						}
						if (z == spriteDepth - 1 || vc->getVoxel(x, y, z + 1).color == 0) {
							buildSide(&vertices, x, y, z, color, VoxelSide::back);
							sideCount++;
						}
					}
				}
			}
		}
	}
	// Set size and type
	m.size = sideCount * 4;
	if (outline)
		m.type = ShaderType::outline;
	else
		m.type = color;
	// Build mesh if any voxels exist
	if (m.size > 0)
	{
		if(outline)
			m.buffer = N3s3d::createBufferFromOverlayVertices(&outlineVertices, m.size);
		else
			m.buffer = N3s3d::createBufferFromColorVertices(&vertices, m.size);	
	}
	return m;
}

void SpriteMesh::buildZMeshes(shared_ptr<VoxelCollection> vc, ShaderType type)
{
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			int i = getArrayIndexFromXY(x, y, 8);
			int zArray[32];
			for (int z = 0; z < 32; z++)
			{
				zArray[z] = vc->getVoxel(x, y, z).color;
			}
			if(type == color)
				zMeshes[i] = GameData::getSharedMesh(zArray, color);
			else
				outlineZMeshes[i] = GameData::getSharedMesh(zArray, outline);
		}
	}
}

void SpriteMesh::rebuildZMesh(int x, int y)
{

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
	mesh.size = sideCount * 4;
	mesh.type = color;
	mesh.buffer = N3s3d::createBufferFromColorVertices(&vertices, mesh.size);
	return mesh;
}

VoxelMesh buildOutlineZMesh(int zArray[32])
{
	VoxelMesh mesh = VoxelMesh();
	vector<OverlayVertex> vertices;
	int sideCount = 0;
	for (int z = 0; z < spriteDepth; z++)
	{
		int color = zArray[z];
		// See if the voxel is not empty
		if (color == 1)
		{
			// Draw left, right, top, and bottom edges, which should be visible on Z-Meshes no matter what
			buildSideOutline(&vertices, 0, 0, z, VoxelSide::left);
			buildSideOutline(&vertices, 0, 0, z, VoxelSide::right);
			buildSideOutline(&vertices, 0, 0, z, VoxelSide::top);
			buildSideOutline(&vertices, 0, 0, z, VoxelSide::bottom);
			sideCount += 4;
			// See if front and/or back are exposed, and add sides if sos
			if (z == 0 || zArray[z - 1] == 0)
			{
				buildSideOutline(&vertices, 0, 0, z, front);
				sideCount++;
			}
			if (z == 31 || zArray[z + 1] == 0)
			{
				buildSideOutline(&vertices, 0, 0, z, back);
				sideCount++;
			}
			// TODO: Also add > evaluation for semi-transparent voxels in the future
		}
	}
	mesh.size = sideCount * 4;
	mesh.type = outline;
	mesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, mesh.size);
	return mesh;
}

void buildSide(vector<ColorVertex> * vertices, int x, int y, int z, int color, VoxelSide side) {
	// Translate voxel-space xyz into model-space based on pixel size in 3d space, phew
	float xf = x * pixelSizeW;
	float yf = y * -pixelSizeW;
	float zf = z * pixelSizeW;
	// Init 4 vertices
	ColorVertex v1, v2, v3, v4;
	// Set all colors, subtract one to align with shader array
	v1.Color = color - 1;
	v2.Color = color - 1;
	v3.Color = color - 1;
	v4.Color = color - 1;
	// Add UV coordinates
	v1.Uv = { 0.0f, 1.0f };
	v2.Uv = { 1.0f, 1.0f };
	v3.Uv = { 1.0f, 0.0f };
	v4.Uv = { 0.0f, 0.0f };
	// Set normal
	v1.Normal = sideNormals[side];
	v2.Normal = sideNormals[side];
	v3.Normal = sideNormals[side];
	v4.Normal = sideNormals[side];

	// Switch based on side
	switch (side) {
	case VoxelSide::left:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		break;
	case VoxelSide::right:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf, 1.0f);
		break;
	case VoxelSide::top:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		break;
	case VoxelSide::bottom:
		v1.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		break;
	case VoxelSide::front:
		v1.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf, 1.0f);
		break;
	case VoxelSide::back:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		break;
	}
	vertices->push_back(v1);
	vertices->push_back(v2);
	vertices->push_back(v3);
	vertices->push_back(v4);
}

void buildSideOutline(vector<OverlayVertex> * vertices, int x, int y, int z, VoxelSide side) {
	// Translate voxel-space xyz into model-space based on pixel size in 3d space, phew
	float xf = x * pixelSizeW;
	float yf = y * -pixelSizeW;
	float zf = z * pixelSizeW;
	// Init 4 vertices
	OverlayVertex v1, v2, v3, v4;

	// Switch based on side
	switch (side) {
	case VoxelSide::left:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		break;
	case VoxelSide::right:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf, 1.0f);
		break;
	case VoxelSide::top:
		v1.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		break;
	case VoxelSide::bottom:
		v1.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		break;
	case VoxelSide::front:
		v1.Pos = XMFLOAT4(xf, yf, zf, 1.0f);
		v2.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf, 1.0f);
		v3.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf, 1.0f);
		v4.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf, 1.0f);
		break;
	case VoxelSide::back:
		v1.Pos = XMFLOAT4(xf + pixelSizeW, yf, zf + pixelSizeW, 1.0f);
		v2.Pos = XMFLOAT4(xf, yf, zf + pixelSizeW, 1.0f);
		v3.Pos = XMFLOAT4(xf, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		v4.Pos = XMFLOAT4(xf + pixelSizeW, yf - pixelSizeW, zf + pixelSizeW, 1.0f);
		break;
	}
	vertices->push_back(v1);
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
			int uniqueCount = i - spritesIgnored;
			// Add the data of the sprite we're going to generate to the map of duplicates
			previousSprites.insert(spriteData);
			// Build mesh and add to list
			shared_ptr<SpriteMesh> mesh = make_shared<SpriteMesh>(current);
			mesh->id = uniqueCount;
			meshes[uniqueCount] = mesh;
			// Build the static sprite
			shared_ptr<VirtualSprite> vSprite = make_shared<VirtualSprite>(spriteData, meshes[uniqueCount]);
			vSprite->appearancesInRomChr.push_back(i);	// Record where this sprite occured in CHR data
			vSprite->id = uniqueCount;
			sprites[i] = vSprite;
			spritesByChrData[spriteData] = vSprite;
		}
		else
		{
			// Record appreance in CHR data but don't bother building new sprite/mesh
			spritesByChrData[spriteData]->appearancesInRomChr.push_back(i);
			spritesIgnored++;
		}
	}
}

GameData::GameData(char* data, json j)
{
	// Get ROM metadata
	romInfo = getRomInfo(data);
	// Load meshes
	for each (json jM in j["meshes"])
	{
		// Get ID from (probably padded) string
		const string s = jM["id"];
		int id = stoi(s);
		// Build mesh from JSON and add to map of meshes by ID
		shared_ptr<SpriteMesh> mesh = make_shared<SpriteMesh>(jM);
		mesh->id = id;
		meshes[id] = mesh;
	}
	// Load VirtualSprites
	for each (json jS in j["sprites"])
	{
		shared_ptr<VirtualSprite> sprite = make_shared<VirtualSprite>(jS, meshes);
		sprites[sprite->id] = sprite;
		spritesByChrData[sprite->chrData] = sprite;
	}
	int test = 0;
	// TEST TEST TEST!!!f
	//DynamicMesh dM;
	//dM.mesh = meshes[507];
	//ConditionSet cS;
	//Condition c;
	//c.parameter = palette_num;
	//c.comparison = DynamicComparison::equal;
	//c.variables[0] = 0;
	//cS.conditions.push_back(c);
	//c.parameter = relative_nametable;
	//c.variables[0] = -1;
	//c.variables[1] = 0;
	//c.variables[2] = 359;
	//cS.conditions.push_back(c);
	//c.variables[0] = 1;
	//c.variables[2] = 360;
	//cS.conditions.push_back(c);
	//dM.conditionSets.push_back(cS);
	//sprites[292]->dynamicMeshes.push_back(dM);
	//sprites[292]->hasDynamicMesh = true;
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

VoxelMesh GameData::getSharedMesh(int zArray[32], ShaderType type)
{
	// Create simple string hash from the voxels
	string hash = "";
	for (int z = 0; z < 31; z++)
	{
		hash += digits[zArray[z]];
		hash += '.';
	}
	hash += digits[zArray[31]];
	// Return mesh if it exists, otherwise create
	if (type == color)
	{
		if (sharedPaletteMeshes.count(hash) > 0)
		{
			// Increase number of references
			sharedPaletteMeshes[hash].referenceCount++;
			// Return reference
			return sharedPaletteMeshes[hash].mesh;
		}
		else
		{
			SharedMesh m = SharedMesh();
			m.mesh = buildZMesh(zArray);
			m.referenceCount++;
			sharedPaletteMeshes[hash] = m;
			return m.mesh;
		}
	}
	else
	{
		if (sharedOutlineMeshes.count(hash) > 0)
		{
			// Increase number of references
			sharedOutlineMeshes[hash].referenceCount++;
			// Return reference
			return sharedOutlineMeshes[hash].mesh;
		}
		else
		{
			SharedMesh m = SharedMesh();
			m.mesh = buildOutlineZMesh(zArray);
			m.referenceCount++;
			sharedOutlineMeshes[hash] = m;
			return m.mesh;
		}
	}
}

void GameData::releaseSharedMesh(string hash, ShaderType type)
{
	// Branch based on shader type
	if (type == color)
	{
		// Make sure this actually exists
		if (sharedPaletteMeshes.count(hash) > 0)
		{
			sharedPaletteMeshes[hash].referenceCount--;
			// Delete this mesh if it is no longer in use
			if (sharedPaletteMeshes[hash].referenceCount <= 0)
			{
				sharedPaletteMeshes[hash].mesh.buffer->Release();
				sharedPaletteMeshes.erase(hash);
			}
		}
	}
	else
	{
		// Make sure this actually exists
		if (sharedOutlineMeshes.count(hash) > 0)
		{
			sharedOutlineMeshes[hash].referenceCount--;
			// Delete this mesh if it is no longer in use
			if (sharedOutlineMeshes[hash].referenceCount <= 0)
			{
				sharedOutlineMeshes[hash].mesh.buffer->Release();
				sharedOutlineMeshes.erase(hash);
			}
		}
	}
}

void GameData::unload()
{
	// Release all full meshes
	for (auto kvp : meshes)
	{
		if (kvp.second->meshExists)
		{
			kvp.second->mesh.buffer->Release();
		}
	}
	// Release all partial meshes
	for (auto kv : sharedPaletteMeshes)
	{
		if (kv.second.mesh.buffer != nullptr)
			kv.second.mesh.buffer->Release();
	}
	for (auto kv : sharedOutlineMeshes)
	{
		if (kv.second.mesh.buffer != nullptr)
			kv.second.mesh.buffer->Release();
	}
	meshes.clear();
	sprites.clear();
	sharedPaletteMeshes.clear();
	sharedOutlineMeshes.clear();
}

json GameData::getJSON()
{
	json j;
	j["info"]["prgSize"] = romInfo.prgSize;
	j["info"]["chrSize"] = romInfo.chrSize;
	j["info"]["mapper"] = romInfo.mapper;
	j["info"]["trainer"] = romInfo.trainer;
	j["info"]["playChoice10"] = romInfo.playChoice10;
	j["info"]["PAL;"] = romInfo.PAL;
	for each (auto kvp in meshes)
	{
		// Create string from ID, pad to 5 digits to enforce proper order
		j["meshes"].push_back(kvp.second->getJSON());
	}
	for each (auto kvp in sprites)
	{
		j["sprites"].push_back(kvp.second->getJSON());
	}
	return j;
}

void GameData::initMaps()
{
	for (int i = 0; i < parameter_size; i++)
		parameterNameMap[parameterNames[i]] = (DynamicParameter)i;
	for (int i = 0; i < comparison_size; i++)
		comparisonNameMap[comparisonNames[i]] = (DynamicComparison)i;
}

bool SpriteMesh::buildMesh()
{
	// Release old buffer, if this mesh is being changed
	if (meshExists)
		mesh.buffer->Release();

	// Build the main mesh
	mesh = buildMeshFromVoxelCollection(voxels, false);

	if(mesh.size > 0)
	{
		meshExists = true;
		buildZMeshes(voxels, color);
		// Build outline mesh, if set to a color
		if (outlineColor >= 0)
		{
			shared_ptr<VoxelCollection> outlineVoxels = makeOutlineVoxelCollection(voxels, false);
			outlineMesh = buildMeshFromVoxelCollection(outlineVoxels, true);
			buildZMeshes(outlineVoxels, outline);
		}
		return true;
	}
	else
	{
		meshExists = false;
		mesh.buffer = nullptr;
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

VoxelCollection::VoxelCollection(json j)
{
	for (int z = 0; z < 32; z++)
	{
		for (int y = 0; y < 8; y++)
		{
			string yString = j[y];
			for (int x = 0; x < 8; x++)
			{
				int8_t color = yString[z * 9 + x] - 48;
				setVoxel(x, y, z, { color });
			}
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
	if (x >= 0 && x < 8 && y >= 0 && y < 8 && z >= 0 && z < 32)
	{
		int voxelSlot = x + (y * spriteWidth) + (z * (spriteHeight * spriteWidth));
		voxels[voxelSlot] = v;
	}
}

void VoxelCollection::clear()
{
	for (int i = 0; i < spriteSize; i++)
	{
		voxels[i].color = 0;
	}
}

json VoxelCollection::getJSON()
{
	json j;
	string s[8] = { "", "", "", "", "", "", "", "" };
	// The data is staggered in a weird way
	// I want a json string for each row, where the sprite's 8 y-axis layers are on top of eachother the same way in text
	for (int z = 0; z < 32; z++)
	{
		for (int y = 0; y < 8; y++)
		{
			for (int x = 0; x < 8; x++)
			{
				s[y] += intChars[getVoxel(x, y, z).color];
			}
			s[y] += '|'; // Horizontal divider
		}
	}
	for (int y = 0; y < 8; y++)
	{
		s[y].pop_back();	// Trim last | delimiter
		j.push_back(s[y]);	// Push string to json structure
	}
	return j;
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

bool getBitLeftSide(char byte, int position)
{
	return (byte << position) & 0x80;
}

string getPaddedStringFromInt(int value, int length)
{
	string s;
	for (int i = to_string(value).length(); i < length; i++)
	{
		s += '0';
	}
	s.append(to_string(value));
	return s;
}

int charToInt(char c)
{
	// Thanks Niels: http://stackoverflow.com/questions/17261798/converting-a-hex-string-to-a-byte-array
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

//VirtualSprite::VirtualSprite()
//{
//}

VirtualSprite::VirtualSprite(string chrData, shared_ptr<SpriteMesh> defaultMesh) : chrData(chrData), defaultMesh(defaultMesh)
{
}

VirtualSprite::VirtualSprite(json j, map<int, shared_ptr<SpriteMesh>> meshes)
{
	// Grab the ID from padded string int
	id = stoi(j["id"].get<string>());
	// Get the CHR data (convert from hex string to byte/char string)
	getChrStringFromText(j["chrData"]);
	// Snag default mesh from passed map
	int mesh = j["defaultMesh"];
	defaultMesh = meshes[mesh];
	// Get dynamic meshes, if specified
	if (j.count("dynamicMeshes") > 0)
	{
		for each(json dJ in j["dynamicMeshes"])
		{
			DynamicMesh dynamicMesh = DynamicMesh(dJ, id, meshes);
			if (dynamicMesh.mesh != nullptr && dynamicMesh.specifiedMesh > 0)
			{
				dynamicMeshes.push_back(dynamicMesh);
			}
		}
		if(!dynamicMeshes.empty())
			hasDynamicMesh = true;
	}
	// Get other values
	for each (int i in j["appearancesInRomChr"])
	{
		appearancesInRomChr.push_back(i);
	}
	description = j["description"].get<string>();
}

void VirtualSprite::render(int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop)
{
	defaultMesh->render(x, y, palette, mirrorH, mirrorV, crop);
}

void VirtualSprite::renderOAM(shared_ptr<PpuSnapshot> snapshot, int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop)
{
	// TODO: Check dynamic stuff
	defaultMesh->render(x, y, palette, mirrorH, mirrorV, crop);
}

void VirtualSprite::renderNametable(shared_ptr<PpuSnapshot> snapshot, int x, int y, int palette, int nametableX, int nametableY, Crop crop)
{
	// TODO: Check dynamic stuff
	defaultMesh->render(x, y, palette, false, false, crop);
}

json VirtualSprite::getJSON()
{
	json j;
	j["id"] = getPaddedStringFromInt(id, 5);
	j["chrData"] = serializeChrDataAsText();
	j["appearancesInRomChr"] = appearancesInRomChr;
	j["description"] = description;
	j["defaultMesh"] = defaultMesh->id;
	for each (DynamicMesh d in dynamicMeshes)
	{
		j["dynamicMeshes"].push_back(d.getJSON());
	}
	return j;
}

string VirtualSprite::serializeChrDataAsText()
{
	string s = "";
	for (int i = 0; i < 16; i++)
	{
		unsigned char c = chrData[i];
		int a = (int)c >> 4;
		int b = (int)c & 15;
		s += hexCodes[a];
		s += hexCodes[b];
	}
	return s;
}

void VirtualSprite::getChrStringFromText(string s)
{
	for (int i = 0; i < 16; i++)
	{
		int position = i * 2;
		// Grab both characters for each value
		int a = charToInt(s[position]);
		int b = charToInt(s[position + 1]);
		// Assuming "a" is leftmost 4 bits, multiply by 16 and add B to get original "byte"
		chrData += ((a * 16) + b);
	}
}

void SpriteMesh::render(int x, int y, int palette, bool mirrorH, bool mirrorV, Crop crop)
{
	if (meshExists)
	{
		// Check if we're cropping
		if (crop.zeroed())
		{
			// Not cropped, render normally
			N3s3d::selectPalette(palette);
			float posX, posY;
			posX = -1.0f + (pixelSizeW * x);
			posY = 1.0f - (pixelSizeW * y);
			N3s3d::updateMirroring(mirrorH, mirrorV);
			if (mirrorH)
				posX += (pixelSizeW * 8);
			if (mirrorV)
				posY -= (pixelSizeW * 8);
			N3s3d::updateWorldMatrix(posX, posY, 0);
			// See if we should be drawing any outlines
			if (outlineColor >= 0)
			{
				// Render mesh, writing to stencil buffer with unique id
				N3s3d::renderMesh(&mesh);
				// Cache outline to be rendered later
				N3s3d::cacheOutlineMesh(&outlineMesh, palette, outlineColor, posX, posY, mirrorH, mirrorV);
			}
			else
			{
				N3s3d::renderMesh(&mesh);
			}
		}
		else
		{
			// Render the cropped version with zMeshes
			N3s3d::selectPalette(palette);
			int height = 8 - crop.top - crop.bottom;
			int width = 8 - crop.left - crop.right;
			for (int posY = 0; posY < height; posY++)
			{
				for (int posX = 0; posX < width; posX++)
				{
					// Grab different zMeshes based on mirroring
					int meshX, meshY;
					if (mirrorH)
						meshX = 7 - (posX + crop.left);
					else
						meshX = posX + crop.left;
					if (mirrorV)
						meshY = 7 - (posY + crop.top);
					else
						meshY = posY + crop.top;
					if (zMeshes[(meshY * 8) + meshX].buffer != nullptr)
					{
						N3s3d::updateWorldMatrix(-1.0f + ((x + posX) * pixelSizeW), 1.0f - ((y + posY) * pixelSizeW), 0);
						N3s3d::renderMesh(&zMeshes[(meshY * 8) + meshX]);
					}
				}
			}
		}
	}
}

void SpriteMesh::moveLayer(int oldX, int oldY, int oldZ, int newX, int newY, int newZ, bool copy)
{
	if (oldZ != newZ)
	{
		if (oldZ > 0 && oldZ < 32 && newZ > 0 && newZ < 32)
		{
			// Move layer on Z axis
			for (int x = 0; x < 8; x++)
				for (int y = 0; y < 8; y++)
					setVoxel({ x, y, newZ }, voxels->getVoxel(x, y, oldZ).color);
			if (!copy)
			{
				for (int x = 0; x < 8; x++)
					for (int y = 0; y < 8; y++)
						setVoxel({ x, y, oldZ }, 0);
			}
		}
		// Rebuild the mesh
		buildMesh();
	}
	else if (oldX != newX)
	{
		if (oldX > 0 && oldX < 8 && newZ > 0 && newX < 8)
		{
			// Move layer on X axis
			for (int z = 0; z < 32; z++)
				for (int y = 0; y < 8; y++)
					setVoxel({ newX, y, z }, voxels->getVoxel(oldX, y, z).color);
			if (!copy)
			{
				for (int z = 0; z < 32; z++)
					for (int y = 0; y < 8; y++)
						setVoxel({ oldX, y, z }, 0);
			}
		}
		// Rebuild the mesh
		buildMesh();
	}
	else if (oldY != newY)
	{
		if (oldX > 0 && oldX < 8 && newZ > 0 && newX < 8)
		{
			// Move layer on X axis
			for (int z = 0; z < 32; z++)
				for (int y = 0; y < 8; y++)
					setVoxel({ newX, y, z }, voxels->getVoxel(oldX, y, z).color);
			if (!copy)
			{
				for (int z = 0; z < 32; z++)
					for (int y = 0; y < 8; y++)
						setVoxel({ oldX, y, z }, 0);
			}
		}
		// Rebuild the mesh
		buildMesh();
	}
}

json SpriteMesh::getJSON()
{
	json j;
	j["id"] = getPaddedStringFromInt(id, 5);
	j["voxels"] = voxels->getJSON();
	return j;
}

void SpriteMesh::setOutline(int o)
{
	if (o >= -1 && o <= 3)
		// See if outline was set at all previously
		if (outlineColor < 0 && o >= 0)
		{
			outlineColor = o;
			if (meshExists)
			{
				shared_ptr<VoxelCollection> vc = makeOutlineVoxelCollection(voxels, false);
				outlineMesh = buildMeshFromVoxelCollection(vc, true);
				buildZMeshes(vc, outline);
			}

		}
		else
		{
			outlineColor = o;
		}
}

Condition::Condition(json j, int spriteID)
{
	// See if the parameter is valid, return bad condition if not
	if (j.count("parameter") > 0 && parameterNameMap.find(j["parameter"]) != parameterNameMap.end())
		parameter = parameterNameMap[j["parameter"]];
	else
	{
		N3sConsole::writeLine(debug_editor, "Bad dynamic parameter in JSON, sprite ID: " + to_string(spriteID));
		variables[0] = -9999;
		return;
	}
	// See if the comparison is valid, return bad condition if not
	if (j.count("comparison") > 0 && comparisonNameMap.find(j["comparison"]) != comparisonNameMap.end())
		comparison = comparisonNameMap[j["comparison"]];
	else
	{
		N3sConsole::writeLine(debug_editor, "Bad dynamic comparison in JSON, sprite ID: " + to_string(spriteID));
		variables[0] = -9999;
		return;
	}
	// Grab 
	int variablesGiven = j["variables"].size();
	if (variablesGiven > 0)
	{
		for (int i = 0; i < 4; i++)
		{
			if (i < variablesGiven)
			{
				if (j["variables"][i].is_number_integer())
				{
					variables[i] = j["variables"][i];
				}
				else
				{
					variables[0] = -9999;
					N3sConsole::writeLine(debug_editor, "Variable #" + to_string(i) + " is bad in sprite #" + to_string(spriteID));
				}
			}
			else
			{
				variables[i] = 0;
			}
		}
	}
	else
	{
		N3sConsole::writeLine(debug_editor, "Bad dynamic comparison in JSON, sprite ID: " + to_string(spriteID));
		variables[0] = -9999;
		return;
	}
}

json Condition::getJSON()
{
	json j;
	j["parameter"] = parameterNames[(int)parameter];
	j["comparison"] = comparisonNames[(int)comparison];
	for (int i = 0; i < 4; i++)
	{
		j["variables"][i] = variables[i];
	}
	return j;
}

bool Condition::compareAll(int result[4])
{
	switch (parameter)
	{
	case color_id:
		return(compare(variables[1], result[0]));
	case palette_num:
		return(compare(variables[0], result[0]));
	case relative_nametable:
		return(compare(variables[2], result[0]));
	default:
		return false;
	}
}

bool Condition::compare(int expected, int result)
{
	switch (comparison)
	{
	case DynamicComparison::equal:
		if (expected == result) return true; else return false;
		break;
	case DynamicComparison::not_equal:
		if (expected != result) return true; else return false;
		break;
	case DynamicComparison::less:
		if (expected < result) return true; else return false;
		break;
	case DynamicComparison::less_or_equal:
		if (expected <= result) return true; else return false;
		break;
	case DynamicComparison::greater:
		if (expected > result) return true; else return false;
		break;
	case DynamicComparison::greater_or_equal:
		if (expected > result) return true; else return false;
		break;
	}
	return false;
}

DynamicMesh::DynamicMesh(json j, int spriteNumber, map<int, shared_ptr<SpriteMesh>> meshes)
{
	// Make sure a mesh is specified, and that it exists
	if (j.count("mesh") > 0 && j["mesh"].is_number_integer())
	{
		specifiedMesh = j["mesh"];
		if (specifiedMesh >= 0 && specifiedMesh < meshes.size())
		{
			mesh = meshes[specifiedMesh];
		}
		else {
			N3sConsole::writeLine(debug_editor, "One of the dynamic meshes for sprite #" + to_string(spriteNumber) + " points to a non-existant mesh.");
			specifiedMesh = -1;
			return;
		}
	}
	else
	{	
		// Return error mesh 
		N3sConsole::writeLine(debug_editor, "One of the dynamic meshes for sprite #" + to_string(spriteNumber) + " had no conditions.");
		specifiedMesh = -1;
		return;
	}
	// Check that conditionSets are specified
	if (j.count("conditionSets") > 0)
	{
		int setCount = j["conditionSets"].size();
		if (setCount > 0)
		{
			for (int i = 0; i < setCount; i++)
			{
				ConditionSet set;
				int conditionCount = j["conditionSets"][i].size();
				for (int c = 0; c < conditionCount; c++)
				{
					Condition condition = Condition(j["conditionSets"][i][c], 0);
					// If the condition works, add it
					if (condition.variables[0] != -9999)
					{
						set.conditions.push_back(condition);
					}
				}
				conditionSets.push_back(set);
			}
		}
	}
	else
	{
		// Nothing to load
		N3sConsole::writeLine(debug_editor, "One of the dynamic meshes specified had no conditions.");
		specifiedMesh = -1;
		return;
	}
}

json DynamicMesh::getJSON()
{
	json j;
	j["mesh"] = mesh->id;
	for each(ConditionSet set in conditionSets)
	{
		json setJson;
		for each(Condition c in set.conditions)
			setJson.push_back(c.getJSON());
		j["conditionSets"].push_back(setJson);
	}
	return j;
}
