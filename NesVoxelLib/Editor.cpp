#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"

shared_ptr<Scene> scenes[8];

int selectedScene = 0;
int selectedSprite;
bool editing;
Camera mainCamera;

XMVECTOR mouseVector;
XMFLOAT3 zIntersect;

int mousePixelX;
int mousePixelY;

SceneSelector sceneSelector;
PaletteSelector paletteSelector;

bool Editor::mouseAvailable;
XMVECTOR Editor::mouseVector;

void getCoordinatesFromZIntersection()
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

void Editor::init()
{
	mouseAvailable = true;
	// Set camera to default position
	mainCamera.SetPosition(0, 0, -2);
	for (int i = 0; i < 8; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
	zIntersect = { 0.0f, 0.0f, 0.0f };
	scenes[0]->setBackgroundSprite(15, 15, { 3,1,0,0,false,false });
	scenes[0]->addOAMSprite({ 3, 0, 0, 0, false, false });
	scenes[0]->addOAMSprite({ 3, 0, 200, 171, false, false });
	scenes[0]->addOAMSprite({ 3, 0, 133, 117, false, false });
	scenes[0]->addOAMSprite({ 3, 0, 32, 32, false, false });
}

void Editor::update()
{
	mouseAvailable = sceneSelector.update(mouseAvailable);
	mouseAvailable = paletteSelector.update(mouseAvailable, scenes[selectedScene]);
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
	// If mouse has moved and is available, calculate highlighted items
	if(InputState::keyboardMouse->hasMouseMoved() && mouseAvailable)
		scenes[selectedScene]->updateHighlight2d(mousePixelX, mousePixelY);
}

void Editor::render()
{
	shared_ptr<Scene> scene = scenes[selectedScene];
	N3s3d::updateMatricesWithCamera(&mainCamera);
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Render the scene
	scene->render(true, true);
	// Render overlays
	scene->renderOverlays(true, true);
	// TEST render NT highlight
	if (scene->highlight.selectedIndex >= 0)
	{
		N3s3d::setShader(overlay);
		N3s3d::setRasterFillState(false);
		N3s3d::setDepthBufferState(false);
		// See if it's a OAM or nametable highlight
		if (scene->highlight.getHighlightedOAM() >= 0)
		{
			SceneSprite s = scene->sprites[scene->highlight.getHighlightedOAM()];
			Overlay::setColor(0.0f, 1.0f, 0.0f, 1.0f);
			Overlay::drawSpriteSquare(s.x, s.y);
		}
		else if (scene->highlight.getHighlightedNT() >= 0)
		{
			int xNT = floor(mousePixelX / 8);
			int yNT = floor(mousePixelY / 8);
			Overlay::setColor(1.0f, 0.0f, 0.0f, 0.5f);
			Overlay::drawSpriteSquare(xNT * 8, yNT * 8);
		}
		N3s3d::setRasterFillState(true);
	}

	// Render GUI
	N3s3d::setGuiProjection();
	sceneSelector.render();
	paletteSelector.render(scene);
}

void Editor::setScene(int s)
{
	selectedScene = s;
}

Hue Editor::getBackgroundColor()
{
	return scenes[selectedScene]->getSelectedPalette()->getBackgroundColor();
}
