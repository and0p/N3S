#include "stdafx.h"
#include "Scene.hpp"

Scene::Scene()
{
	// Clear BG with all "blank" (-1) sprites
	for (int i = 0; i < sceneWidth * sceneHeight; i++)
	{
		bg[i] = { -1, 0, false, false };	
	}
}

void Scene::render(bool renderBackground, bool renderOAM)
{
	// Update palette in video card
	palettes[selectedPalette].updateShaderPalette();
	// Render background, if enabled
	if (renderBackground)
	{
		for (int y = 0; y < sceneHeight; y++)
		{
			int yCalc = y * sceneWidth;
			for (int x = 0; x < sceneWidth; x++)
			{
				SceneSprite sprite = bg[yCalc + x];
				// Only render non-empty spots, which are 0 or greater
				if (sprite.meshNum >= 0)
				{
					N3sApp::gameData->meshes[sprite.meshNum]->render(x * 8, y * 8, sprite.palette, sprite.mirrorH, sprite.mirrorV, { 0,0,0,0 });
				}
			}
		}
	}
	// Render OAM, if enabled
	if (renderOAM)
	{
		N3s3d::setDepthStencilState(true, true, false);
		for each(SceneSprite s in sprites)
		{
			N3sApp::gameData->meshes[s.meshNum]->render(s.x, s.y, s.palette, s.mirrorH, s.mirrorV, { 0, 0, 0, 0 });
		}
	}
}

void Scene::renderOverlays(bool drawBackgroundGrid, bool drawOamHighlights)
{
	// Render background grid, if enabled
	N3s3d::setShader(overlay);
	N3s3d::setRasterFillState(false);
	if (drawBackgroundGrid)
	{
		Overlay::setColor(1.0f, 0.0f, 0.0f, 0.1f);
		Overlay::drawNametableGrid(0, 0);
		Overlay::drawNametableGrid(32, 0);
		Overlay::drawNametableGrid(0, 30);
		Overlay::drawNametableGrid(32, 30);
	}
	if (drawOamHighlights)
	{
		Overlay::setColor(1.0f, 1.0f, 1.0f, 1.0f);
		for each(SceneSprite s in sprites)
		{
			Overlay::drawSpriteSquare(s.x, s.y);
		}
	}
	N3s3d::setRasterFillState(true);
}

void Scene::setBackgroundSprite(int x, int y, SceneSprite sprite)
{
	bg[y * sceneWidth + x] = sprite;
}

void Scene::addOAMSprite(SceneSprite s)
{
	sprites.push_back(s);
}

void Scene::createSceneFromCurrentSnapshot()
{
	int scrollX = N3sApp::snapshot->scrollSections[0].x;
	int scrollY = N3sApp::snapshot->scrollSections[0].y;
	int nameTable = N3sApp::snapshot->scrollSections[0].nameTable;
	// todo: adjust scroll x and y by nametable selection

	// Grab all background tiles
	for (int y = 0; y < sceneHeight; y++)
	{
		int yName = floor(y / 30);
		int yCalc = y * sceneWidth;
		for (int x = 0; x < sceneWidth; x++)
		{
			int xName = floor(x / 32);
			NameTableTile t = N3sApp::snapshot->background.getTile(x, y, yName + xName);
			SceneSprite s = { t.tile, t.palette, false, false }; // NOPE gotta get the mesh #
		}
	}
}

N3sPalette * Scene::getSelectedPalette()
{
	return &palettes[selectedPalette];
}

void Scene::selectNextPalette()
{
	selectedPalette++;
	if (selectedPalette > 7)
		selectedPalette = 0;
}

void Scene::selectPreviousPalette()
{
	selectedPalette--;
	if (selectedPalette < 0)
		selectedPalette = 7;
}

void Scene::updateHighlight2d(int x, int y, bool highlightOAM, bool highlightNametable)
{
	// Clear previous highlight
	highlight.clear();
	// See if any OAM sprites intersect selection
	if (highlightOAM)
	{
		for (int i = 0; i < sprites.size(); i++)
		{
			SceneSprite s = sprites[i];
			if (x >= s.x && x < s.x + 8 && y >= s.y && y < s.y + 8)
				highlight.highlightedSpriteIndices.push_back(i);
		}
	}
	// See if any part of the background intersects selection
	if (highlightNametable)
	{
		if (x >= 0 && x < scenePixelWidth && y >= 0 && y < scenePixelHeight)
			highlight.highlightedBackgroundIndex = floor(y / 8) * 64 + floor(x / 8);
	}
	// Set the index
	if (highlight.highlightedBackgroundIndex >= 0 || highlight.highlightedSpriteIndices.size() > 0)
		highlight.selectedIndex = 0;
}

void Highlight::clear()
{
	highlightedSpriteIndices.clear();
	highlightedBackgroundIndex = -1;
	selectedIndex = -1;
}

int Highlight::getHighlightedOAM()
{
	if (selectedIndex >= 0 && selectedIndex < highlightedSpriteIndices.size())
		return highlightedSpriteIndices[selectedIndex];
	else
		return -1;
}

int Highlight::getHighlightedNT()
{
	if (selectedIndex == highlightedSpriteIndices.size())
		return highlightedBackgroundIndex;
	else
		return -1;
}