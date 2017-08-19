#include "stdafx.h"
#include "GameView.hpp"
#include "N3sConsole.hpp"

shared_ptr<OrbitCamera> gameCamera = make_shared<OrbitCamera>(OrbitCamera());

void GameView::update()
{
	float camXRotation = InputState::functions[cam_left].value + InputState::functions[cam_right].value * -1;
	float camYRotation = InputState::functions[cam_up].value - InputState::functions[cam_down].value;
	float camXMove = (InputState::functions[cam_move_left].value * -0.03f) + (InputState::functions[cam_move_right].value * 0.03f);
	float camYMove = InputState::functions[cam_move_up].value * 0.03f - InputState::functions[cam_move_down].value * 0.03f;
	float camZoom = InputState::functions[cam_pan_in].value * 0.05f - InputState::functions[cam_pan_out].value * 0.05f;
	gameCamera->AdjustPosition(camXMove, camYMove, 0.0f);
	gameCamera->AdjustRotation(camXRotation, camYRotation, 0.0f);
	gameCamera->adjustZoom(camZoom);

	// Update camera position
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
		gameCamera->AdjustRotation(xRot, yRot, 0.0f);
	}
	if (InputState::keyboardMouse->mouseButtons[middle_mouse].state > 0)
	{
		float xPos = InputState::keyboardMouse->mouseDeltaX / 400;
		float yPos = InputState::keyboardMouse->mouseDeltaY / 400;
		gameCamera->AdjustPosition(-xPos, yPos, 0.0f);
	}
	// Update camera zoom
	gameCamera->adjustZoom((float)InputState::keyboardMouse->calculatedWheelDelta / 10);

	//if (inputState->gamePads[0].buttonStates[brb])
	//{
	//	//gameCamera->SetPosition(0, 0, 0);
	//	//gameCamera->SetRotation(0, 0, 0);
	//}

	//N3sConsole::update();
}

void GameView::render()
{
	// "Render" camera to matrices
	gameCamera->Render();
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Render scene
	N3s3d::setShader(color);
	N3s3d::updateMatricesWithCamera(gameCamera);
	N3s3d::updateWorldMatrix(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	N3s3d::updateMirroring(true, true);				// TODO add N3s3d function to reset mirroring
	N3s3d::updateMirroring(false, false);
	updatePalette();
	if (N3sApp::snapshot->mask.renderSprites)
	renderSprites();
	if (N3sApp::snapshot->mask.renderBackground)
	renderNameTables();
}

shared_ptr<OrbitCamera> GameView::getCamera()
{
	return gameCamera;
}

void GameView::parseInput()
{
}

void renderSprites()
{
	for (int i = 0; i < 64; i++) {
		OamSprite sprite = N3sApp::snapshot->sprites[i];
		int x = sprite.x;
		int y = sprite.y;
		int tile = sprite.tile;
		// Branch on 8x16 mode
		if (N3sApp::snapshot->ctrl.spriteSize16x8)
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
			shared_ptr<VirtualSprite> vSprite = N3sApp::virtualPatternTable->getSprite(tile);
			// Render the first sprite
			renderSprite(vSprite, x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
			// Render the second sprite, which swaps place with vertical flip
			if (sprite.vFlip)
				renderSprite(N3sApp::virtualPatternTable->getSprite(tile + 1), x, y - 8, sprite.palette, sprite.hFlip, sprite.vFlip);
			else
				renderSprite(N3sApp::virtualPatternTable->getSprite(tile + 1), x, y + 8, sprite.palette, sprite.hFlip, sprite.vFlip);
		}
		else
		{
			// Select tile based on pattern table in CTRL register
			if (tile > 255 && !N3sApp::snapshot->getOAMSelectAtScanline(y))
			{
				tile -= 256;
			}
			else if (tile < 256 && N3sApp::snapshot->getOAMSelectAtScanline(y))
			{
				tile += 256;
			}
			renderSprite(N3sApp::virtualPatternTable->getSprite(tile), x, y, sprite.palette, sprite.hFlip, sprite.vFlip);
		}
	}
}

void renderSprite(shared_ptr<VirtualSprite> vSprite, int x, int y, int palette, bool flipX, bool flipY)
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
		if(vSprite != nullptr)
			vSprite->renderOAM(N3sApp::snapshot, x, y, palette, flipX, flipY, crop);
	}
}


void updatePalette()
{
	N3sApp::snapshot->palette.updateShaderPalette();
}

void renderNameTables()
{
	// Reset tile mirroring, as Nametable cannot use it
	N3s3d::updateMirroring(false, false);
	// Render each scroll section
	for (ScrollSection scrollSection : N3sApp::snapshot->scrollSections)
	{
		renderScrollSection(scrollSection);
	}
}

void renderScrollSection(ScrollSection section)
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

void renderRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect)
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
		tile = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
		if (patternSelect)
			tile += 256;
		palette = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
		N3sApp::virtualPatternTable->getSprite(tile)->renderNametable(N3sApp::snapshot, x, y, palette, tileX + i, tileY, crop);
		x += 8 - xOffset;
		i++;
		// Render middle sprites
		crop.left = 0;
		for (i; i < 32; i++)
		{
			tile = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
			if (patternSelect)
				tile += 256;
			palette = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
			N3sApp::virtualPatternTable->getSprite(tile)->renderNametable(N3sApp::snapshot, x, y, palette, tileX + i, tileY, crop);
			x += 8;
		}
		// Render partial last sprite
		crop.right = 8 - xOffset;
		tile = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
		if (patternSelect)
			tile += 256;
		palette = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
		N3sApp::virtualPatternTable->getSprite(tile)->renderNametable(N3sApp::snapshot, x, y, palette, tileX + i, tileY, crop);
	}
	else
	{
		for (int i = 0; i < 32; i++)
		{
			tile = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).tile;
			if (patternSelect)
				tile += 256;
			palette = N3sApp::snapshot->background.getTile(tileX + i, tileY, nameTable).palette;
			N3sApp::virtualPatternTable->getSprite(tile)->renderNametable(N3sApp::snapshot, x, y, palette, tileX + i, tileY, crop);
			x += 8;
		}
	}
}
