#include "stdafx.h"
#include "Overlay.hpp"

VoxelMesh characterMeshes[characterCount];

void Overlay::init()
{
	for (int i = 0; i < 2; i++)
	{
		characterMeshes[i] = createMeshFromBitmapCharacter(bitmapCharacters[i]);
	}
}

void Overlay::unload()
{
	for (int i = 0; i < 2; i++)
	{
		characterMeshes[i].buffer->Release();
	}
}

void Overlay::drawString(int x, int y, int scale, string s)
{

}

VoxelMesh createMeshFromBitmapCharacter(BitmapCharacter bitmap)
{
	vector<OverlayVertex> vertices;
	int squareCount = 0;
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
				// Increment how many squares (6 vertices) the mesh will be
				squareCount++;
			}
		}
	}
	VoxelMesh mesh;
	mesh.size = squareCount * 6;
	mesh.type = overlay;
	mesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, mesh.size);
	return mesh;
	return VoxelMesh();
}