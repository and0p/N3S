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
	VxlUtil::updateMatricesWithCamera(&camera);
	VxlUtil::updateWorldMatrix(0.0f, 0.0f, 0.0f);
	VxlUtil::updateMirroring(false, false);
	updatePalette();
	renderSprites();
	renderNameTables();
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
				gameData->sprites[tile].renderPartial(x, y, sprite.palette, offset, sprite.hFlip, sprite.vFlip);
			}
			else
				gameData->sprites[tile].render(x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
		}
	}
}

void VxlApp::updatePalette()
{
	float palette[72];
	for (int p = 0; p < 8; p++)
	{
		for (int h = 0; h < 3; h++)
		{
			palette[(p * 9) + (h * 3)] = VxlUtil::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->palette.palettes[p].colors[h]).red;
			palette[(p * 9) + (h * 3) + 1] = VxlUtil::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->palette.palettes[p].colors[h]).green;
			palette[(p * 9) + (h * 3) + 2] = VxlUtil::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->palette.palettes[p].colors[h]).blue;
		}
	}
	Hue hue = VxlUtil::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->palette.palettes[0].colors[2]);
	VxlUtil::updatePalette(palette);
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
		gameData->sprites[tile].renderPartial(x, y, 0, { 0, xOffset, 0, 0 }, false, false);
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
				gameData->sprites[tile].render(x, y, 0, false, false);
			}
			x += 8;
		}
		// Render parital last sprite
		tile = snapshot->background.getTile(nametableX + i, nametableY, nameTable).tile;
		if (tile < 0)
			tile = 512 + tile;
		else
			tile += 256;
		gameData->sprites[tile].renderPartial(x, y, 0, { 0, 0, 0, 8 - xOffset }, false, false);
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
				gameData->sprites[tile].render(x, y, 0, false, false);
			}
			x += 8;
		}
	}
}