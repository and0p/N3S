#include "stdafx.h"
#include "Overlay.hpp"

// Overlay / GUI meshes
VoxelMesh characterMeshes[characterCount];

// Misc editor meshes
VoxelMesh voxelPreviewMesh;		// Single voxel
VoxelMesh voxelGridMesh;		// 8x8 grid, voxel-size
VoxelMesh voxelGridMeshLong;	// 32x8 grid, voxel-size
VoxelMesh nametableGridMesh;	// 32x30 grid, sprite-size
VoxelMesh spriteSquareMesh;		// 2D box for flat sprite selection and highlights
VoxelMesh rectangleMesh;		// 1x1 rectangle, for resizing and drawing

int getScreenX(int x)
{
	return x - (N3s3d::viewport.Width / 2);
}

int getScreenY(int y)
{
	return (N3s3d::viewport.Height / 2) - y;
}

int getScreen0romBottom(int y)
{
	return (N3s3d::viewport.Width / 2) + y;
}

void Overlay::init()
{
	for (int i = 0; i < characterCount; i++)
	{
		characterMeshes[i] = createMeshFromBitmapCharacter(bitmapCharacters[i]);
	}
	buildVoxelPreviewMesh();
	buildGridMeshes();
	buildRectangleMesh();
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
	// Get screen-space coordinates
	int screenX = getScreenX(x);
	int screenY = getScreenY(y);
	int currentX = screenX;
	int currentY = screenY;
	int characterSize = 8 * scale;
	// Iterate over string chars
	for (int i = 0; i < s.length(); i++)
	{
		int c = s[i];
		// Newline
		if(c == 10)
		{
			currentY -= characterSize;
			currentX = screenX;
		}
		else if (c > 32 && c < 32 + characterCount)	// 32 is space
		{
			// Offset so that char ! is 0, like in character mesh array
			c -= 33;
			// Update world matrix
			N3s3d::updateWorldMatrix(currentX, currentY, 0, 0, 0, 0, scale);
			// Render character
			N3s3d::renderMesh(&characterMeshes[c]);
			// Move position over by 1 character
			currentX += characterSize;
		}
		else
		{
			currentX += characterSize;
		}
	}
}

void Overlay::drawVoxelPreview(int x, int y, int z)
{
	// Get true coordinates
	float xf = -1.0f + (pixelSizeW * x);
	float yf = 1.0f - (pixelSizeW * y);
	float zf = (float)z * pixelSizeW;
	// Update world matrix
	N3s3d::updateWorldMatrix(xf, yf, zf);
	// Render character
	N3s3d::renderMesh(&voxelPreviewMesh);
}

void Overlay::drawVoxelGrid(int spriteX, int spriteY, int voxelPos, gridOrientation orientation)
{
	// Set raster mode to wireframe
	N3s3d::setRasterFillState(false);
	float x, y, z;
	// Branch based on orientation
	switch (orientation) {
	case xAxis:
		// Get true world coords
		x = -1.0f + (spriteX * pixelSizeW);
		y = 1.0f - (spriteY * pixelSizeW);
		z = voxelPos * pixelSizeW;
		// Update world transform based on position and orientation
		N3s3d::updateWorldMatrix(x, y, z);
		// Render the mesh
		N3s3d::renderMesh(&voxelGridMesh);
		break;
	case yAxis:
		// Get true world coords
		x = -1.0f + (spriteX * pixelSizeW);
		y = 1.0f - (spriteY * pixelSizeW) - (voxelPos * pixelSizeW);
		z = 0;
		// Update world transform based on position and orientation
		N3s3d::updateWorldMatrix(x, y, z, 90, -90, 0, 1);
		// Render the mesh
		N3s3d::renderMesh(&voxelGridMeshLong);
		break;
	case zAxis:
		// Get true world coords
		x = -1.0f + (spriteX * pixelSizeW) + (voxelPos * pixelSizeW);
		y = 1.0f - (spriteY * pixelSizeW);
		z = 0;
		// Update world transform based on position and orientation
		N3s3d::updateWorldMatrix(x, y, z, 0, -90, 0, 1);
		// Render the mesh
		N3s3d::renderMesh(&voxelGridMeshLong);
		break;
	}
	// Set raster mode back to fille
	N3s3d::setRasterFillState(true);
}

void Overlay::drawNametableGrid(int x, int y)
{
	N3s3d::setRasterFillState(false);
	N3s3d::updateWorldMatrix(x * pixelSizeW * 8, -y * pixelSizeW * 8, pixelSizeW * 16);
	N3s3d::renderMesh(&nametableGridMesh);
	N3s3d::setRasterFillState(true);
}

void Overlay::drawRectangle(int x, int y, int width, int height)
{
	int screenX = getScreenX(x);
	int screenY = getScreenY(y);
	// Update world matrix
	N3s3d::updateWorldMatrix(screenX, screenY, 0, 0, 0, 0, width, height, 1);
	// Render rectangle
	N3s3d::renderMesh(&rectangleMesh);
}

void Overlay::drawRectangleInScene(int x, int y, int z, int width, int height)
{
	N3s3d::updateWorldMatrix(
		-1.0f + x * pixelSizeW, 1.0f - (y * pixelSizeW), z * pixelSizeW,
		0, 0, 0,
		width * pixelSizeW, height * pixelSizeW, 1);
	N3s3d::renderMesh(&rectangleMesh);
}

void Overlay::drawSpriteSquare(int x, int y)
{
	// Get true coordinates
	float xf = -1.0f + (pixelSizeW * x);
	float yf = 1.0f - (pixelSizeW * y);
	// Update world matrix
	N3s3d::updateWorldMatrix(xf, yf, 0.0f);
	// Render character
	N3s3d::renderMesh(&spriteSquareMesh);
}

void Overlay::setColor(int r, int g, int b, int a)
{
	N3s3d::setOverlayColor(r, g, b, a);
}

void Overlay::setColor(float r, float g, float b, float a)
{
	N3s3d::setOverlayColor(r, g, b, a);
}

VoxelMesh createMeshFromBitmapCharacter(BitmapCharacter bitmap)
{
	vector<OverlayVertex> vertices;
	int squareCount = 0;
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (bitmap.pixels[x + (y * 8)] == 1)
			{
				float xf = x;
				float yf = y;
				// Init 4 vertices
				OverlayVertex v1, v2, v3, v4;
				v1.Pos = XMFLOAT4(xf, (yf * -1), 0, 1.0f);
				v2.Pos = XMFLOAT4(xf + 1, (yf * -1), 0, 1.0f);
				v3.Pos = XMFLOAT4(xf + 1, (yf * -1) - 1, 0, 1.0f);
				v4.Pos = XMFLOAT4(xf, (yf * -1) - 1, 0, 1.0f);
				// Push back to end of array
				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v4);
				vertices.push_back(v2);
				vertices.push_back(v3);
				vertices.push_back(v4);
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
}

// This mesh is just a single voxel at standard size,
void buildVoxelPreviewMesh()
{
	voxelPreviewMesh.type = overlay;
	voxelPreviewMesh.size = 36;

	vector<OverlayVertex> vertices;
	OverlayVertex v1, v2, v3, v4;
	// Add left side
	v1.Pos = XMFLOAT4(0, 0, 0 + pixelSizeW, 1.0f);
	v2.Pos = XMFLOAT4(0, 0, 0, 1.0f);
	v3.Pos = XMFLOAT4(0, 0 - pixelSizeW, 0, 1.0f);
	v4.Pos = XMFLOAT4(0, 0 - pixelSizeW, 0 + pixelSizeW, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	// Add right side
	v1.Pos = XMFLOAT4(0 + pixelSizeW, 0, 0, 1.0f);
	v2.Pos = XMFLOAT4(0 + pixelSizeW, 0, 0 + pixelSizeW, 1.0f);
	v3.Pos = XMFLOAT4(0 + pixelSizeW, 0 - pixelSizeW, 0 + pixelSizeW, 1.0f);
	v4.Pos = XMFLOAT4(0 + pixelSizeW, 0 - pixelSizeW, 0, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	// Add top side
	v1.Pos = XMFLOAT4(0, 0, 0 + pixelSizeW, 1.0f);
	v2.Pos = XMFLOAT4(0 + pixelSizeW, 0, 0 + pixelSizeW, 1.0f);
	v3.Pos = XMFLOAT4(0 + pixelSizeW, 0, 0, 1.0f);
	v4.Pos = XMFLOAT4(0, 0, 0, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	// Add bottom side
	v1.Pos = XMFLOAT4(0, 0 - pixelSizeW, 0, 1.0f);
	v2.Pos = XMFLOAT4(0 + pixelSizeW, 0 - pixelSizeW, 0, 1.0f);
	v3.Pos = XMFLOAT4(0 + pixelSizeW, 0 - pixelSizeW, 0 + pixelSizeW, 1.0f);
	v4.Pos = XMFLOAT4(0, 0 - pixelSizeW, 0 + pixelSizeW, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	// Add front side
	v1.Pos = XMFLOAT4(0, 0, 0, 1.0f);
	v2.Pos = XMFLOAT4(0 + pixelSizeW, 0, 0, 1.0f);
	v3.Pos = XMFLOAT4(0 + pixelSizeW, 0 - pixelSizeW, 0, 1.0f);
	v4.Pos = XMFLOAT4(0, 0 - pixelSizeW, 0, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);
	// Add back side
	v1.Pos = XMFLOAT4(0 + pixelSizeW, 0, 0 + pixelSizeW, 1.0f);
	v2.Pos = XMFLOAT4(0, 0, 0 + pixelSizeW, 1.0f);
	v3.Pos = XMFLOAT4(0, 0 - pixelSizeW, 0 + pixelSizeW, 1.0f);
	v4.Pos = XMFLOAT4(0 + pixelSizeW, 0 - pixelSizeW, 0 + pixelSizeW, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	voxelPreviewMesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, 36);
}

void buildGridMeshes()
{
	// Build the 8x8 grid
	voxelGridMesh.type = overlay;
	voxelGridMesh.size = 9 * 6;
	
	vector<OverlayVertex> vertices;
	OverlayVertex v1, v2;

	for (float i = 0; i < 9; i += 1)
	{
		// Add horizontal line
		v1.Pos = XMFLOAT4(0, i * -pixelSizeW, 0, 1.0f);
		v2.Pos = XMFLOAT4(pixelSizeW * 8, i * -pixelSizeW, 0, 1.0f);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v1);
		// Add vertical line
		v1.Pos = XMFLOAT4(i * pixelSizeW, 0, 0, 1.0f);
		v2.Pos = XMFLOAT4(i * pixelSizeW, 8 * -pixelSizeW, 0, 1.0f);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v1);
	}

	voxelGridMesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, voxelGridMesh.size);

	// Build the 32x8 grid, along z-axis by default
	voxelGridMeshLong.type = overlay;
	voxelGridMeshLong.size = (9 + 33) * 3;

	vertices.clear();

	// Add horizontal lines
	for (int i = 0; i < 9; i++)
	{
		v1.Pos = XMFLOAT4(0, i * -pixelSizeW, 0, 1.0f);
		v2.Pos = XMFLOAT4(pixelSizeW * 32, i * -pixelSizeW, 0, 1.0f);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v1);
	}
	// Add vertical lines
	for (int i = 0; i < 33; i++)
	{
		v1.Pos = XMFLOAT4(i * pixelSizeW, 0, 0, 1.0f);
		v2.Pos = XMFLOAT4(i * pixelSizeW, -8 * pixelSizeW, 0, 1.0f);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v1);
	}

	voxelGridMeshLong.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, voxelGridMeshLong.size);

	// Build the nametable grid
	nametableGridMesh.type = overlay;
	nametableGridMesh.size = (31 + 33) * 3;

	vertices.clear();

	float spriteWidth = pixelSizeW * 8;
	float spriteHeight = pixelSizeW * 8;

	// Add horizontal lines
	for (float i = 0; i < 31; i++)
	{
		v1.Pos = XMFLOAT4(-1.0f, 1.0f - (i * spriteHeight), 0, 1.0f);
		v2.Pos = XMFLOAT4(1.0f, 1.0f - (i * spriteHeight), 0, 1.0f);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v1);
	}
	// Add vertical lines
	for (float i = 0; i < 33; i++)
	{
		v1.Pos = XMFLOAT4(-1.0f + (i * spriteWidth), 1.0f, 0, 1.0f);
		v2.Pos = XMFLOAT4(-1.0f + (i * spriteWidth), -1.0f, 0, 1.0f);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v1);
	}

	nametableGridMesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, nametableGridMesh.size);

	// Build sprite square mesh
	spriteSquareMesh.type = overlay;
	spriteSquareMesh.size = (4) * 3;

	vertices.clear();

	// Top
	v1.Pos = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	v2.Pos = XMFLOAT4(spriteWidth, 0.0f, 0.0f, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v1);
	// Left
	v2.Pos = XMFLOAT4(0.0f, -spriteHeight, 0.0f, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v1);
	// Bottom
	v1.Pos = XMFLOAT4(spriteWidth, -spriteHeight, 0.0f, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v1);
	//Right
	v2.Pos = XMFLOAT4(spriteWidth, 0.0f, 0.0f, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v1);

	spriteSquareMesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, spriteSquareMesh.size);
}

void buildRectangleMesh()
{
	rectangleMesh.type = overlay;
	rectangleMesh.size = 6;

	vector<OverlayVertex> vertices;
	OverlayVertex v1, v2, v3, v4;

	// Add front side
	v1.Pos = XMFLOAT4(0, 0, 0, 1.0f);
	v2.Pos = XMFLOAT4(1.0f, 0, 0, 1.0f);
	v3.Pos = XMFLOAT4(1.0f, -1.0f, 0, 1.0f);
	v4.Pos = XMFLOAT4(0, -1.0f, 0, 1.0f);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v4);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	rectangleMesh.buffer = N3s3d::createBufferFromOverlayVertices(&vertices, rectangleMesh.size);
}