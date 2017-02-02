#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"

shared_ptr<Scene> scenes[8];

int selectedScene = 0;
int selectedSprite;
bool editing;
Camera mainCamera;

SceneSelector sceneSelector;
PaletteSelector paletteSelector;

bool Editor::mouseAvailable;

void Editor::init()
{
	mouseAvailable = true;
	// Set camera to default position
	mainCamera.SetPosition(0, 0, -2);
	for (int i = 0; i < 8; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
	scenes[0]->setBackgroundSprite(0, 0, { 1,0,false,false });
	scenes[0]->setBackgroundSprite(1, 0, { 2,0,false,false });
	scenes[0]->setBackgroundSprite(15, 15, { 3,1,false,false });
	scenes[0]->setBackgroundSprite(3, 0, { 4,0,false,false });
	scenes[1]->setBackgroundSprite(0, 0, { 1,0,false,false });
	scenes[1]->setBackgroundSprite(1, 0, { 2,0,false,false });
	scenes[1]->setBackgroundSprite(2, 0, { 3,0,false,false });
	scenes[1]->setBackgroundSprite(3, 0, { 4,0,false,false });
}

void Editor::update()
{
	sceneSelector.update(mouseAvailable);
	paletteSelector.update(mouseAvailable, scenes[selectedScene]->getSelectedPalette());
}

void Editor::render()
{
	// Update view with whatever camera
	mainCamera.Render();
	N3s3d::updateMatricesWithCamera(&mainCamera);
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Render the scene
	scenes[selectedScene]->render();
	// Render GUI
	N3s3d::setDepthBufferState(false);
	N3s3d::setGuiProjection();
	sceneSelector.render();
	paletteSelector.render(scenes[selectedScene]->getSelectedPalette());
}

void Editor::setScene(int s)
{
	selectedScene = s;
}
