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
	// Flip Y and start calculating as positive
	yAtIntersect = fabs(yAtIntersect);
	// Divide each by full size of scene
	xAtIntersect /= sceneDXWidth;
	yAtIntersect /= sceneDXHeight;
	// Get "pixel" position
	mousePixelX = floor(scenePixelWidth * xAtIntersect);
	mousePixelY = floor(scenePixelHeight * yAtIntersect);
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
	scenes[0]->addOAMSprite({ 3, 0, 64, 64, false, false });
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
}

void Editor::render()
{
	N3s3d::updateMatricesWithCamera(&mainCamera);
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Render the scene
	scenes[selectedScene]->render(true, true);
	// Render overlays
	scenes[selectedScene]->renderOverlays(true, true);
	// TEST render NT highlight
	int xNT = floor(mousePixelX / 8);
	int yNT = floor(mousePixelY / 8);
	N3s3d::setShader(overlay);
	N3s3d::setRasterFillState(false);
	N3s3d::setDepthBufferState(false);
	Overlay::setColor(1.0f, 0.0f, 0.0f, 0.5f);
	Overlay::drawSpriteSquare(xNT * 8, yNT * 8);
	N3s3d::setRasterFillState(true);
	// Render GUI
	N3s3d::setGuiProjection();
	sceneSelector.render();
	paletteSelector.render(scenes[selectedScene]);
}

void Editor::setScene(int s)
{
	selectedScene = s;
}

Hue Editor::getBackgroundColor()
{
	return scenes[selectedScene]->getSelectedPalette()->getBackgroundColor();
}
