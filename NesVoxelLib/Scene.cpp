#include "stdafx.h"
#include "Scene.hpp"
#include "Camera.hpp"

XMVECTOR mouseVector;
XMFLOAT3 zIntersect;

int mousePixelX;
int mousePixelY;

Camera mainCamera;

bool mouseCaptured = false;

MouseModifier modifier = no_mod;
MouseFunction mouseFunction = no_func;

Scene::Scene()
{
	// Set camera to default position
	mainCamera.SetPosition(0, 0, -2);
	// Clear BG with all "blank" (-1) sprites
	for (int i = 0; i < sceneWidth * sceneHeight; i++)
	{
		bg[i] = { -1, 0, false, false };	
	}
}

bool Scene::update(bool mouseAvailable)
{
	// Update camera position
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 3;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 3;
		mainCamera.AdjustRotation(xRot, 0.0f, yRot);
	}
	// Update camera math
	mainCamera.Render();
	// Calculate mouse vector and z-intersect
	mouseVector = N3s3d::getMouseVector(&mainCamera, InputState::keyboardMouse->mouseX, InputState::keyboardMouse->mouseY);
	zIntersect = N3s3d::getZIntersection(&mainCamera, InputState::keyboardMouse->mouseX, InputState::keyboardMouse->mouseY);
	getCoordinatesFromZIntersection();
	return updateMouseActions(mouseAvailable);
}

void Scene::render(bool renderBackground, bool renderOAM)
{
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Update camera math
	N3s3d::updateMatricesWithCamera(&mainCamera);
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
					int i = unwrapArrayIndex(x, y, sceneWidth);
					if (selection.selectedBackgroundIndices.count(i) > 0 || highlight.getHighlightedNT() == i)
					{
						N3s3d::setDepthStencilState(true, true, false);
						N3sApp::gameData->meshes[sprite.meshNum]->render(x * 8, y * 8, sprite.palette, sprite.mirrorH, sprite.mirrorV, { 0,0,0,0 });
						N3s3d::setDepthStencilState(true, false, false);
					}
					// See if this tile is selected or highlighted
					N3sApp::gameData->meshes[sprite.meshNum]->render(x * 8, y * 8, sprite.palette, sprite.mirrorH, sprite.mirrorV, { 0,0,0,0 });
				}
			}
		}
	}
	// Render OAM, if enabled
	if (renderOAM)
	{
		for (int i = 0; i < sprites.size(); i++)
		{
			SceneSprite s = sprites[i];
			// See if sprite is highlighted and write to stencil buffer if so
			if (selection.selectedSpriteIndices.count(i) > 0 || highlight.getHighlightedOAM() == i)
			{
				N3s3d::setDepthStencilState(true, true, false);
				N3sApp::gameData->meshes[s.meshNum]->render(s.x, s.y, s.palette, s.mirrorH, s.mirrorV, { 0, 0, 0, 0 });
				N3s3d::setDepthStencilState(true, false, false);
			}
			else
			{
				N3sApp::gameData->meshes[s.meshNum]->render(s.x, s.y, s.palette, s.mirrorH, s.mirrorV, { 0, 0, 0, 0 });
			}
		}
	}
	// Render highlight selections / mouse hover
	N3s3d::setDepthStencilState(false, false, true);
	N3s3d::setGuiProjection();
	Overlay::setColor(1.0f, 1.0f, 1.0f, 0.3f);
	Overlay::drawRectangle(0, 0, 1920, 1080); // TODO: How do I get actual screen size again..?

}

void Scene::renderOverlays(bool drawBackgroundGrid, bool drawOamHighlights)
{
	N3s3d::setShader(overlay);
	N3s3d::setDepthBufferState(false);
	N3s3d::setRasterFillState(false);
	// Update camera math (was probably left at GUI projection after scene rendering)
	N3s3d::updateMatricesWithCamera(&mainCamera);
	// Render background grid, if enabled
	if (drawBackgroundGrid)
	{
		Overlay::setColor(1.0f, 0.0f, 0.0f, 0.1f);
		Overlay::drawNametableGrid(0, 0);
		Overlay::drawNametableGrid(32, 0);
		Overlay::drawNametableGrid(0, 30);
		Overlay::drawNametableGrid(32, 30);
	}
	// Draw OAM highlights, if enabled
	if (drawOamHighlights)
	{
		Overlay::setColor(1.0f, 1.0f, 1.0f, 1.0f);
		for each(SceneSprite s in sprites)
		{
			Overlay::drawSpriteSquare(s.x, s.y);
		}
	}
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

bool Scene::updateMouseActions(bool mouseAvailable)
{
	// If mouse has moved and is available, calculate highlighted items
	if (InputState::keyboardMouse->hasMouseMoved())
		updateHighlight2d(mousePixelX, mousePixelY, true, true);
	// Check left mouse
	if (mouseAvailable || mouseCaptured)
	{
		MouseState state = InputState::keyboardMouse->mouseButtons[left_mouse].state;
		if (state > 0)
			mouseCaptured = true;
		int mouseX = InputState::keyboardMouse->mouseX;
		int mouseY = InputState::keyboardMouse->mouseY;
		if (mouseCaptured)
		{
			// See if this is a new click
			if (state == clicked)
			{
				// Capture mod key
				if (InputState::functions[selection_add].active && InputState::functions[selection_remove].active)
					modifier = mod_intersect;
				else if (InputState::functions[selection_add].active)
					modifier = mod_add;
				else if (InputState::functions[selection_remove].active)
					modifier = mod_remove;
				else
					modifier = no_mod;
				// See if OAM or NT is highlighted at all
					// See if any objects in highlight are also in selection
						// If so, set function to move
						// Otherwise we're selecting in different ways, based on mod key
			}
			if (state == pressed)
			{
				// Clear previous selection when appropriate
				if(modifier == no_mod)
					selection.clear();
				// Check for highlight and add to selection
				if (highlight.getHighlightedOAM() >= 0)
				{
					if (modifier == no_mod || modifier == mod_add)
						selection.selectedSpriteIndices.insert(highlight.getHighlightedOAM());
					else if (modifier == mod_remove)
						selection.selectedSpriteIndices.erase(highlight.getHighlightedOAM());
				}
				else if (highlight.getHighlightedNT() >= 0)
				{
					if (modifier == no_mod || modifier == mod_add)
						selection.selectedSpriteIndices.insert(highlight.getHighlightedNT());
					else if (modifier == mod_remove)
						selection.selectedSpriteIndices.erase(highlight.getHighlightedNT());
				}
			}
			if (state == dragging)
			{
				// If we are already 
			}
		}
		if (!mouseCaptured)
		{
			mouseFunction = no_func;
			modifier = no_mod;
		}
	}
	return true;
}

void Scene::getCoordinatesFromZIntersection()
{
	float xAtIntersect = zIntersect.x;
	float yAtIntersect = zIntersect.y;
	// Normalize top-left of all screens to 0,0
	xAtIntersect += 1.0f;
	yAtIntersect -= 1.0f;
	// Divide each by full size of scene
	xAtIntersect /= sceneDXWidth;
	yAtIntersect /= sceneDXHeight;
	// Get "pixel" position of X
	mousePixelX = floor(scenePixelWidth * xAtIntersect);
	// Get Y, but flip it first (since negative = positive in NES screenspace)
	mousePixelY = floor(scenePixelHeight * (yAtIntersect * -1));
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

bool Highlight::anythingHighlighted()
{
	if (selectedIndex >= 0)
		return true;
	else
		return false;
}

void Selection::clear()
{
	selectedSpriteIndices.clear();
	selectedBackgroundIndices.clear();
}
