#pragma once
#include "N3s3d.hpp"

const static int characterCount = 64;

using namespace std;

class Overlay {
public:
	static void init();
	static void unload();
	static void drawString(int x, int y, int scale, string s);
	static void test();
	static bool shadow;
	static int width;
	static int height;
};

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

VoxelMesh createMeshFromBitmapCharacter(BitmapCharacter bitmap);