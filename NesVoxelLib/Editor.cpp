#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"

shared_ptr<Scene> scenes[8];

int selectedScene = 0;
int selectedSprite;
bool editing;
Camera mainCamera;

XMFLOAT3 zIntersect;

SceneSelector sceneSelector;
PaletteSelector paletteSelector;

bool Editor::mouseAvailable;
XMVECTOR Editor::mouseVector;

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
	// Calculate mouse vector
	
}

void Editor::render()
{
	// Update view with whatever camera
	mainCamera.Render();
	N3s3d::updateMatricesWithCamera(&mainCamera);
	zIntersect = N3s3d::getZIntersection(&mainCamera, InputState::keyboardMouse->mouseX, InputState::keyboardMouse->mouseY);
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Render the scene
	scenes[selectedScene]->render();
	// Test sprite square
	N3s3d::setShader(overlay);
	Overlay::setColor(0.0f, 1.0f, 0.0f, 0.5f);
	N3s3d::setRasterFillState(false);
	Overlay::drawSpriteSquare(24, 24);
	N3s3d::setRasterFillState(true);
	// Render GUI
	N3s3d::setDepthBufferState(false);
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
