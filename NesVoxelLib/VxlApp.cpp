#include "stdafx.h"
#include "VxlApp.h"

VxlApp::VxlApp()
{
	gameData = {};
	emulationPaused = false;
	loaded = false;
	camera = VxlCamera();
	camera.SetPosition(0, 0, -2);
	camera.SetRotation(0, 0, 0);
}

void VxlApp::assignD3DContext(VxlD3DContext context)
{
	VxlUtil::initPipeline(context);
}

void VxlApp::load()
{
	char romPath[] = "mario.nes\0";
	NesEmulator::Initialize(&romPath[0]);
	info = NesEmulator::getGameInfo();
	gameData = std::shared_ptr<VoxelGameData>(new VoxelGameData((char*)info->data));
	virtualPatternTable.load(512, 8, gameData->chrData);
	gameData->grabBitmapSprites(info->data);
	gameData->createSpritesFromBitmaps();
	loaded = true;
}

void VxlApp::update()
{
	//inputState.refreshInput();
	//bool yPressed = inputState.gamePads[0].buttonStates[by];
	//bool bPressed = inputState.gamePads[0].buttonStates[bb];
	//if (bPressed && !pausedThisPress)
	//	emulationPaused = toggleBool(emulationPaused);
	//if(!emulationPaused || (yPressed && !frameAdvanced))
	NesEmulator::ExecuteFrame();
	//if (bPressed)
	//	pausedThisPress = true;
	//else
	//	pausedThisPress = false;
	//if (yPressed)
	//	frameAdvanced = true;
	//else
	//	frameAdvanced = false;
	snapshot.reset(new VxlPPUSnapshot((VxlRawPPU*)NesEmulator::getVRam()));
	virtualPatternTable.update(snapshot->patternTable);
	/*float zoomAmount = inputState.gamePads[0].triggerStates[lTrigger] - inputState.gamePads[0].triggerStates[rTrigger];
	camera.AdjustPosition(inputState.gamePads[0].analogStickStates[lStick].x * 0.05f, inputState.gamePads[0].analogStickStates[lStick].y * 0.05f, zoomAmount * 0.05f);
	camera.AdjustRotation(inputState.gamePads[0].analogStickStates[rStick].x, 0, inputState.gamePads[0].analogStickStates[rStick].y * -1);
	if (inputState.gamePads[0].buttonStates[brb])
	{
		camera.SetPosition(0, 0, -2);
		camera.SetRotation(0, 0, 0);
	}*/
}

void VxlApp::render()
{
	VxlUtil::setIndexBuffer();
	// VxlUtil::setShader(color);
	// camera.SetPosition(0, 0, -2);
	// camera.SetRotation(0, 0, 0);
	// camera.Render();
	// VxlUtil::updateMatricesWithCamera(&camera);
	VxlUtil::updateWorldMatrix(0.0f, 0.0f, 0.0f);
	updatePalette();
	if(snapshot->mask.renderSprites)
		renderSprites();
	 if(snapshot->mask.renderBackground)
		renderNameTables();
}

void VxlApp::updateCameraViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	VxlUtil::updateViewMatrices(view, projection);
}

void VxlApp::updateGameOriginPosition(float x, float y, float z)
{

}

XMVECTORF32 VxlApp::getBackgroundColor()
{
	Hue hue = VxlUtil::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->backgroundColor);
	return{ hue.red, hue.green, hue.blue, 1.0f };
}

void VxlApp::renderSprites()
{
	for (int i = 0; i < 64; i++) {
		OamSprite sprite = snapshot->sprites[i];
		int x = sprite.x;
		int y = sprite.y;
		int tile = virtualPatternTable.getTrueTileIndex(sprite.tile);
		// See if we're in 8x16 sprite mode and this sprite is verticall mirrored, in which case we have to adjust Y accordingly
		if (snapshot->ctrl.spriteSize16x8 && sprite.vFlip)
			y += 8;
		if (y > 0 && y < 240) {
			if (x >= 248 || y >= 232)
			{
				// Use partial rendering
				int width = 8;
				int height = 8;
				if (x >= 248)
					width = 256 - x;
				if (y >= 232)
					height = 240 - y;
				gameData->sprites[tile].renderPartial(x, y, sprite.palette, 0, width, 0, height, sprite.hFlip, sprite.vFlip);
			}
			else
				gameData->sprites[tile].render(x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
		}
		// Render the accompanying vertical sprite if PPU in 8x16 mode
		if (snapshot->ctrl.spriteSize16x8)
		{
			tile++;
			if (sprite.vFlip)
				y -= 8;
			else
				y += 8;
			if (y > 0 && y < 240) {
				if (x >= 248 || y >= 232)
				{
					// Use partial rendering
					int width = 8;
					int height = 8;
					if (x >= 248)
						width = 256 - x;
					if (y >= 232)
						height = 240 - y;
					gameData->sprites[tile].renderPartial(x, y, sprite.palette, 0, width, 0, height, sprite.hFlip, sprite.vFlip);
				}
				else
					gameData->sprites[tile].render(x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
			}
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
	// Get size of section
	int sectionHeight = section.bottom - section.top + 1;
	// Render rows based on section size and yOffset
	int topRowHeight = 8 - yOffset;
	renderRow(section.top, topRowHeight, xOffset, yOffset, section.x, section.y, section.nameTable);
	int yPositionOffset = topRowHeight;
	// Render rest of rows, depending on how many there are
	if (sectionHeight <= 8)
	{
		if (sectionHeight > topRowHeight)
		{
			int bottomRowHeight = sectionHeight - topRowHeight;
			renderRow(section.top + yPositionOffset, bottomRowHeight, xOffset, 0, section.x, section.y + yPositionOffset, section.nameTable);
		}
	}
	else
	{
		// Render all full rows
		int fullRows = floor((sectionHeight - topRowHeight) / 8);
		for (int i = 0; i < fullRows; i++)
		{
			renderRow(section.top + yPositionOffset, 8, xOffset, 0, section.x, section.y + yPositionOffset, section.nameTable);
			yPositionOffset += 8;
		}
		if (yOffset != 0)
		{
			int bottomRowHeight = (sectionHeight - topRowHeight) % 8;
			renderRow(section.top + yPositionOffset, bottomRowHeight, xOffset, 0, section.x, section.y + yPositionOffset, section.nameTable);
		}
	}
}

void VxlApp::renderRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable)
{
	int x = 0;
	NameTableTile nameTableTile;
	int tileX = floor(nametableX / 8);
	int tileY = floor(nametableY / 8);
	// Branch based on whether or not there is any X offset / partial sprite
	if (xOffset > 0)
	{
		int i = 0;
		// Render partial first sprite
		nameTableTile = snapshot->background.getTile(tileX + i, tileY, nameTable);
		gameData->sprites[virtualPatternTable.getTrueTileIndex(nameTableTile.tile)].renderPartial(x, y, nameTableTile.palette, xOffset, (8 - xOffset), yOffset, height, false, false);
		x += 8 - xOffset;
		i++;
		// Render middle sprites
		for (i; i < 32; i++)
		{
			nameTableTile = snapshot->background.getTile(tileX + i, tileY, nameTable);
			if (height < 8)
			{
				// Render partial sprite
				gameData->sprites[virtualPatternTable.getTrueTileIndex(nameTableTile.tile)].renderPartial(x, y, nameTableTile.palette, 0, 8, yOffset, height, false, false);
			}
			else
			{
				gameData->sprites[virtualPatternTable.getTrueTileIndex(nameTableTile.tile)].render(x, y, nameTableTile.palette, false, false);
			}
			x += 8;
		}
		// Render parital last sprite
		nameTableTile = snapshot->background.getTile(tileX + i, tileY, nameTable);
		gameData->sprites[virtualPatternTable.getTrueTileIndex(nameTableTile.tile)].renderPartial(x, y, nameTableTile.palette, 0, xOffset, yOffset, height, false, false);
	}
	else
	{
		// Render all full sprites
		for (int i = 0; i < 32; i++)
		{
			NameTableTile nameTableTile = snapshot->background.getTile(tileX + i, tileY, nameTable);
			if (height < 8)
			{
				// Render partial sprite
				gameData->sprites[virtualPatternTable.getTrueTileIndex(nameTableTile.tile)].renderPartial(x, y, nameTableTile.palette, 0, 8, yOffset, height, false, false);
			}
			else
			{
				gameData->sprites[virtualPatternTable.getTrueTileIndex(nameTableTile.tile)].render(x, y, nameTableTile.palette, false, false);
			}
			x += 8;
		}
	}
}