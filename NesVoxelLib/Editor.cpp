#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"

shared_ptr<Scene> scenes[8];

int selectedScene = 0;
int selectedSprite;
bool editing;

SceneSelector sceneSelector;
PaletteSelector paletteSelector;

bool Editor::mouseAvailable;

void Editor::init()
{
	mouseAvailable = true;
	for (int i = 0; i < 8; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
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
	scenes[selectedScene]->update(mouseAvailable);
}

void Editor::parseInputForScene(bool mouseAvailable)
{
	
}

void Editor::render()
{
	shared_ptr<Scene> scene = scenes[selectedScene];

	// Render the scene
	scene->render(true, true);
	// Render overlays
	scene->renderOverlays(true, true);
	// Render GUI
	N3s3d::setDepthBufferState(false);
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

void Editor::updateTempScene(shared_ptr<PpuSnapshot> snapshot)
{
	scenes[7].reset(new Scene(snapshot));
}