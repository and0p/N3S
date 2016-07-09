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
	camera.SetPosition(0.65f, +0.2f, -2.0f);
	camera.SetRotation(-15.0f, -0.3f, 0);
	camera.Render();
	//camera.SetPosition(-1.8f, -0.5f, -0.5f);
	//camera.SetRotation(+60.0f, 0.0f, 0);
	//camera.Render();
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
		OamSprite sprite = snapshot->sprites[i];
		int x = sprite.x;
		int y = sprite.y;
		int tile = sprite.tile;
		if (y > 0 && y < 240) {
			// Use partial rendering as needed
			if (x >= 248 || y >= 232)
			{
				Sides offset = Sides();
				if (x >= 248)
					offset.right = 8 - (256 - x);
				if (y >= 232)
					offset.bottom = 8 - (240 - y);
				gameData->sprites[tile].renderPartial(x, y, offset, sprite.hFlip, sprite.vFlip);
			}
			else
				gameData->sprites[tile].render(x, y, sprite.hFlip, sprite.vFlip);
		}
	}
}

void VxlApp::renderNameTables()
{
	// Reset tile mirroring, as Nametable cannot use it
	VxlUtil::updateMirroring(false, false);
	// Render each scroll section
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
		// Render partial first sprite
		int tile = snapshot->background.getTile(nametableX + i, nametableY, nameTable).tile;
		if (tile < 0)
			tile = 512 + tile;
		else
			tile += 256;
		gameData->sprites[tile].renderPartial(x, y, { 0, xOffset, 0, 0 }, false, false);
		x += 8 - xOffset;
		i++;
		// Render middle sprites
		for (i; i < 32; i++)
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
		// Render parital last sprite
		tile = snapshot->background.getTile(nametableX + i, nametableY, nameTable).tile;
		if (tile < 0)
			tile = 512 + tile;
		else
			tile += 256;
		gameData->sprites[tile].renderPartial(x, y, { 0, 0, 0, 8 - xOffset }, false, false);
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