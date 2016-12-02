#include "stdafx.h"
#include "Overlay.hpp"

void Overlay::drawString(int x, int y, int scale, string s)
{

}

VoxelMesh Overlay::createMeshFromBitmapCharacter(BitmapCharacter bitmap)
{
	vector<OverlayVertex> vertices;
	int squarecount = 0;
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			if (bitmap.pixels[x + (y * 8)] == 1)
			{
				// Init 4 vertices
				OverlayVertex v1, v2, v3, v4;
				v1.Pos = XMFLOAT4(x, (y * -1), 0, 1.0f);
				v2.Pos = XMFLOAT4(x + 1, (y * -1), 0, 1.0f);
				v3.Pos = XMFLOAT4(x + 1, (y * -1) - 1, 0, 1.0f);
				v4.Pos = XMFLOAT4(x, (y * -1) - 1, 0, 1.0f);
			}
		}
	}
	return VoxelMesh();
}