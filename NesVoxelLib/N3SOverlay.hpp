#pragma once
#include "VxlUtil.h"

const static int characterCount = 64;

using namespace std;

struct BitmapCharacter
{
	int pixels[64];
};

static BitmapCharacter bitmapCharacters[2] =
{
	{
		0,0,0,1,1,0,0,0,
		0,0,1,1,1,1,0,0, 
		0,0,1,1,1,1,0,0, 
		0,0,1,1,1,1,0,0, 
		0,0,0,1,1,0,0,0, 
		0,0,0,1,1,0,0,0, 
		0,0,0,0,0,0,0,0, 
		0,0,0,1,1,0,0,0 
	},
	{
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0
	}
};

class N3SOverlay {
public:
	static void init();
	static void unload();
	static void drawString(int x, int y, int scale, string s);
	static bool shadow;
private:
	static VoxelMesh characterMeshes[characterCount];
	VoxelMesh createMeshFromBitmapCharacter(BitmapCharacter bitmap);
};