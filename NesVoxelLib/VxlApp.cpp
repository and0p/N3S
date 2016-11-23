#include "stdafx.h"
#include "VxlApp.h"
#include <time.h>

extern SoundDriver *newDirectSound();

VxlApp::VxlApp()
{
	this->hwnd = hwnd;
	gameData = {};
	emulationPaused = false;
	loaded = false;
	muted = false;
	camera.SetPosition(0, 0, -2);
	camera.SetRotation(0, 0, 0);
	SoundDriver * drv = 0;
}

void VxlApp::assignD3DContext(VxlD3DContext context)
{
	VxlUtil::initPipeline(context);
	
}

void VxlApp::initDirectAudio(HWND hwnd)
{
	audioEngine = newDirectSound();
	audioEngine->init(hwnd, 44100);
	NesEmulator::audioEngine = audioEngine;
	audioEngine->resume();
}


void VxlApp::load(char *path)
{
	if (loaded)
		unload();
	NesEmulator::Initialize(path);
	info = NesEmulator::getGameInfo();
	// See if a matching N3S file exists with same name as ROM
	if (true)
	{
		// TODO: Doing it in a really dumb way at the moment, stripping nes and replacing with n3s
		string s = (path);
		int length = s.length();
		s[length - 2] = '3';
		ifstream n3sFile(s.c_str());
		json input(n3sFile);
		s = input.dump(4);
		//n3sFile >> input;
		n3sFile.close();
		gameData = shared_ptr<GameData>(new GameData((char*)info->data, input));
	}
	else
	{
		// TODO: Ask if user wants to find N3S file, generate data, or cancel

	}
	// gameData = std::shared_ptr<GameData>(new GameData((char*)info->data));
	virtualPatternTable = shared_ptr<VirtualPatternTable>(new VirtualPatternTable(gameData));
	// gameData->grabBitmapSprites(info->data);
	// gameData->createSpritesFromBitmaps();
	loaded = true;
	unpause();
}

void VxlApp::unload()
{
	if (loaded)
	{
		pause();
		gameData->unload();
		gameData.reset();
		virtualPatternTable.reset();
		camera.SetPosition(0, 0, -2);
		camera.SetRotation(0, 0, 0);
		loaded = false;
	}
}

void VxlApp::reset()
{
	NesEmulator::reset();
}

void VxlApp::update(bool runThisNesFrame)
{
	if (loaded)
	{
		inputState.checkGamePads();
		inputState.refreshInput();
		bool yPressed = inputState.gamePads[0].buttonStates[by];
		bool bPressed = inputState.gamePads[0].buttonStates[bb];
		if (bPressed && !pausedThisPress && !emulationPaused)
		{
			pausedThisPress = true;
			pause();
		}
		else if (bPressed && !pausedThisPress && emulationPaused)
		{
			unpause();
			pausedThisPress = true;
		}
		if (!bPressed)
				pausedThisPress = false;
		if (!emulationPaused || (yPressed && !frameAdvanced) && !runThisNesFrame)
			NesEmulator::ExecuteFrame();
		if (yPressed)
			frameAdvanced = true;
		else
			frameAdvanced = false;
		if (!runThisNesFrame)
		{
			snapshot.reset(new VxlPPUSnapshot((VxlRawPPU*)NesEmulator::getVRam()));
			virtualPatternTable->update(snapshot->patternTable);
		}
		float zoomAmount = inputState.gamePads[0].triggerStates[lTrigger] - inputState.gamePads[0].triggerStates[rTrigger];
		camera.AdjustPosition(inputState.gamePads[0].analogStickStates[lStick].x * 0.05f, inputState.gamePads[0].analogStickStates[lStick].y * 0.05f, zoomAmount * 0.05f);
		camera.AdjustRotation(inputState.gamePads[0].analogStickStates[rStick].x, 0, inputState.gamePads[0].analogStickStates[rStick].y * -1);
		if (inputState.gamePads[0].buttonStates[brb])
		{
			camera.SetPosition(0, 0, -2);
			camera.SetRotation(0, 0, 0);
		}
		// TEST output json
		//string output = gameData->getJSON();
		//ofstream myfile;
		//myfile.open("json.n3s");
		//myfile << output;
		//myfile.close();	
	}
}

void VxlApp::render()
{
	if (loaded)
	{
		VxlUtil::setShader(color);
		camera.Render();
		VxlUtil::updateMatricesWithCamera(&camera);
		VxlUtil::updateWorldMatrix(0.0f, 0.0f, 0.0f);
		VxlUtil::updateMirroring(true, true);
		VxlUtil::updateMirroring(false, false);
		updatePalette();
		if (snapshot->mask.renderSprites)
			renderSprites();
		if (snapshot->mask.renderBackground)
			renderNameTables();
	}
}

void VxlApp::pause()
{
	emulationPaused = true;
	audioEngine->pause();
}

void VxlApp::unpause()
{
	emulationPaused = false;
	audioEngine->resume();
}

void VxlApp::setMute(bool mute)
{
	muted = mute;
	if (muted)
		audioEngine->pause();
	else
		audioEngine->resume();
}

void VxlApp::updateCameraViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	VxlUtil::updateViewMatrices(view, projection);
}

void VxlApp::updateGameOriginPosition(float x, float y, float z)
{

}

void VxlApp::recieveKeyInput(int key, bool down)
{
	if (down)
		inputState.keyboardState.setDown(key);
	else
		inputState.keyboardState.setUp(key);
}

XMVECTORF32 VxlApp::getBackgroundColor()
{
	Hue hue;
	if (loaded)
		hue = VxlUtil::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->backgroundColor);
	else
		hue = { 0.0f, 0.0f, 0.0f };
	return{ hue.red, hue.green, hue.blue, 1.0f };
}

void VxlApp::renderSprites()
{
	for (int i = 0; i < 64; i++) {
		OamSprite sprite = snapshot->sprites[i];
		int x = sprite.x;
		int y = sprite.y;
		int tile = sprite.tile;
		// Branch on 8x16 mode
		if (snapshot->ctrl.spriteSize16x8)
		{
			// In 8x16, pattern table selection is specified by first bit.
			// Since you can only select even (in base 0) tiles it is free for this purpose.
			// Actual tile selection is bits 1-7.
			int patternSelection = tile & 1;
			// Nestopia tends to give tiles by absolute value, but seemingly only some of the time?
			// So we need to correct to be sure
			if (patternSelection && tile < 256)
				tile += 256;
			else if (!patternSelection && tile > 255)
				tile -= 256;
			if (patternSelection)
				tile--;
			// Flipped sprites in 8x16 also swap places on the Y axis
			if (sprite.vFlip)
				y += 8;
			// Get true tile #
			shared_ptr<VirtualSprite> vSprite = virtualPatternTable->getSprite(tile);
			// Render the first sprite
			renderSprite(vSprite, x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
			// Render the second sprite, which swaps place with vertical flip
			if(sprite.vFlip)
				renderSprite(virtualPatternTable->getSprite(tile + 1), x, y - 8, sprite.palette, sprite.hFlip, sprite.vFlip);
			else
				renderSprite(virtualPatternTable->getSprite(tile + 1), x, y + 8, sprite.palette, sprite.hFlip, sprite.vFlip);
		}
		else
		{
			// Select tile based on pattern table in CTRL register
			if (tile > 255 && !snapshot->getOAMSelectAtScanline(y))
			{
				tile -= 256;
			}
			else if (tile < 256 && snapshot->getOAMSelectAtScanline(y))
			{
				tile += 256;
			}
			renderSprite(virtualPatternTable->getSprite(tile), x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
		}
	}
}

void VxlApp::renderSprite(shared_ptr<VirtualSprite> vSprite, int x, int y, int palette, bool flipX, bool flipY)
{
	// Check that sprite is on renderable line (sprites with Y of 0 are ignored) or X position
	if (y > 0 && y < 240 && x >= 0 && x < 256) {
		// See if this needs to be cropped at all due to right / bottom screen edge
		Crop crop = { 0, 0, 0 ,0 };
		if (x > 248)
			crop.right = 256 - x;
		if (y > 232)
			crop.bottom = 240 - y;
		// Render
		vSprite->renderOAM(snapshot, x, y, palette, flipX, flipY, crop);
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
	// Get size of section
	int sectionHeight = section.bottom - section.top + 1;
	// Render rows based on section size and yOffset
	int topRowHeight = 8 - yOffset;
	renderRow(section.top, topRowHeight, xOffset, yOffset, section.x, section.y, section.nameTable, section.patternSelect);
	int yPositionOffset = topRowHeight;
	// Render rest of rows, depending on how many there are
	if (sectionHeight <= 8)
	{
		if (sectionHeight > topRowHeight)
		{
			int bottomRowHeight = sectionHeight - topRowHeight;
			renderRow(section.top + yPositionOffset, bottomRowHeight, xOffset, 0, section.x, section.y + yPositionOffset, section.nameTable, section.patternSelect);
		}
	}
	else
	{
		// Render all full rows
		int fullRows = floor((sectionHeight - topRowHeight) / 8);
		for (int i = 0; i < fullRows; i++)
		{
			renderRow(section.top + yPositionOffset, 8, xOffset, 0, section.x, section.y + yPositionOffset, section.nameTable, section.patternSelect);
			yPositionOffset += 8;
		}
		if (yOffset != 0)
		{
			int bottomRowHeight = (sectionHeight - topRowHeight) % 8;
			renderRow(section.top + yPositionOffset, bottomRowHeight, xOffset, 0, section.x, section.y + yPositionOffset, section.nameTable, section.patternSelect);
		}
	}
}

void VxlApp::renderRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect)
{
	int x = 0;
	int tileX = floor(nametableX / 8);
	int tileY = floor(nametableY / 8);
	int tile;
	int palette;
	Crop crop = { yOffset, xOffset, 8 - yOffset - height, 0 };
	// Branch based on whether or not there is any X offset / partial sprite
	if (xOffset > 0)
	{
		int i = 0;
		// Render partial first sprite
		tile = snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
		if (patternSelect)
			tile += 256;
		palette = snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
		virtualPatternTable->getSprite(tile)->renderNametable(snapshot, x, y, palette, tileX + i, tileY, crop);
		x += 8 - xOffset;
		i++;
		// Render middle sprites
		crop.left = 0;
		for (i; i < 32; i++)
		{
			tile = snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
			if (patternSelect)
				tile += 256;
			palette = snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
			virtualPatternTable->getSprite(tile)->renderNametable(snapshot, x, y, palette, tileX + i, tileY, crop);
			x += 8;
		}
		// Render partial last sprite
		crop.right = 8 - xOffset;
		tile = snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
		if (patternSelect)
			tile += 256;
		palette = snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
		virtualPatternTable->getSprite(tile)->renderNametable(snapshot, x, y, palette, tileX + i, tileY, crop);
	}
	else
	{
		for (int i = 0; i < 32; i++)
		{
			tile = snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
			if (patternSelect)
				tile += 256;
			palette = snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
			virtualPatternTable->getSprite(tile)->renderNametable(snapshot, x, y, palette, tileX + i, tileY, crop);
			x += 8;
		}
	}
}