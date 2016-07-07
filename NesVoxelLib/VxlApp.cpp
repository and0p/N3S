#include "stdafx.h"
#include "VxlApp.h"

VxlApp::VxlApp()
{
	
	gameData = std::shared_ptr<VoxelGameData>(new VoxelGameData(512, 1));
	camera = VxlCamera();
}

void VxlApp::assignD3DContext(VxlD3DContext context)
{
	VxlUtil::initPipeline(context);
}

void VxlApp::load()
{
	NesEmulator::Initialize();
	info = NesEmulator::getGameInfo();
	gameData->grabBitmapSprites(info->data, 32784);
	gameData->createSpritesFromBitmaps();
	gameData->buildAllMeshes();
}

void VxlApp::update()
{
	NesEmulator::ExecuteFrame();
	snapshot.reset(new VxlPPUSnapshot((VxlRawPPU*)NesEmulator::getVRam()));
}

void VxlApp::render()
{
	VxlUtil::updateGameTexture(NesEmulator::getPixelData());
	camera.SetPosition(0.65f, 0.0f, -2.0f);
	camera.SetRotation(-15.0f, 0.0f, 0);
	camera.Render();
	VxlUtil::updateMatricesWithCamera(&camera);
	VxlUtil::updateWorldMatrix(0.0f, 0.0f, 0.0f);
	VxlUtil::updateMirroring(false, false);
	renderSprites();
	renderNameTables();
	//for (int y = 0; y < 30; y++)
	//{
	//	for (int x = 0; x < 32; x++)
	//	{
	//		float posX, posY;
	//		posX = -1.0f + (pixelSizeW * x * 8);
	//		posX -= (pixelSizeW * snapshot->ppuScroll);
	//		posY = 1.0f - (pixelSizeH * y * 8);
	//		VxlUtil::updateWorldMatrix(posX, posY, -0.2f);
	//		int tile = snapshot->background.getTile(x, y, 0).tile;
	//		if (tile < 0)
	//			tile = 512 + tile;
	//		else
	//			tile += 256;
	//		if (gameData->sprites[tile].meshExists)
	//		{
	//			VxlUtil::renderMesh(gameData->sprites[tile].mesh);
	//		}
	//	}
	//}
}

void VxlApp::renderSprites()
{
	for (int i = 0; i < 64; i++) {
		int x = snapshot->sprites[i].x;
		int y = snapshot->sprites[i].y;
		int tile = snapshot->sprites[i].tile;
		if (y > 0 && y < 240) {
			gameData->sprites[tile].render(x, y, false, false);
		}
	}
}

void VxlApp::renderNameTables()
{
	// renderScrollSection(snapshot->scrollSections[0]);
	for (ScrollSection scrollSection : snapshot->scrollSections)
	{
		renderScrollSection(scrollSection);
	}
}

void VxlApp::renderScrollSection(ScrollSection section)
{
	// Get offset of top-left pixel within top-left sprite referenced
	int xOffset = section.x % 8;
	int yOffset = section.y % 8;
	// Count height of section
	int sectionHeight = section.bottom - section.top + 1;
	// Figure out how much to clip the sprites on the edges, if at all
	Sides offset = { 0, 0, 0, 0 };
	if (yOffset > 0)
	{
		offset.top = 8 - yOffset;
		offset.bottom = (sectionHeight - yOffset) % 8;
	}
	if (xOffset > 0)
	{
		offset.left = 8 - xOffset;
		offset.right = (256 - xOffset) % 8;
	}
	// Count how many rows of different sprites there will be
	int rows = floor((sectionHeight - offset.top - offset.bottom) / 8);
	if (yOffset > 0)
	{
		if (yOffset + sectionHeight > 8)
			rows += 2;
		else
			rows++;
	}
	// Start rendering
	int y = section.top;
	int yNameTableIncrement = 0;
	for (int r = 0; r < rows; r++)
	{
		// Render partial if it's the first or last row, and there's also a Y offset
		if (r == 0 && offset.top != 0)
		{
			renderRow(y, offset.top, xOffset, section.x, section.y + (r * 8), section.nameTable);
			y += offset.top;
		}
		else if (r == rows - 1 && offset.bottom != 0)
		{
			renderRow(y, offset.bottom, xOffset, section.x, section.y + (r * 8), section.nameTable);
		}
		else
		{
			renderRow(y, 8, xOffset, floor(section.x / 8), floor(section.y / 8) + r, section.nameTable);
			y += 8;
		}
	}
}

void VxlApp::renderRow(int y, int height, int xOffset, int nametableX, int nametableY, int nameTable)
{

	int x = 0;
	// Branch based on whether or not there is any X offset / partial sprite
	if (xOffset > 0)
	{
		int i = 0;
		// TODO: Render partial first sprite
		x += 8;
		i++;
		// Render middle sprites
		for (i; i < 31; i++)
		{
			if (height < 8)
			{
				// Render partial sprite
			}
			else
			{
				int tile = snapshot->background.getTile(nametableX + i, nametableY, nameTable).tile;
				if (tile < 0)
					tile = 512 + tile;
				else
					tile += 256;
				gameData->sprites[tile].render(x - xOffset, y, false, false);
			}
			x += 8;
		}
		// TODO: Render parital last sprite
	}
	else
	{
		// Render all full sprites
		for (int i = 0; i < 32; i++)
		{
			if (height < 8)
			{
				// Render partial sprite
			}
			else
			{
				int tile = snapshot->background.getTile(nametableX + i, nametableY, nameTable).tile;
				if (tile < 0)
					tile = 512 + tile;
				else
					tile += 256;
				gameData->sprites[tile].render(x, y, false, false);
			}
			x += 8;
		}
	}
}