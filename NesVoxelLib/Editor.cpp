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

	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Render the scene
	scene->render(true, true);
	// Render overlays
	N3s3d::setDepthBufferState(false);
	scene->renderOverlays(true, true);
	//// TEST render NT highlight
	//if (scene->highlight.selectedIndex >= 0)
	//{
	//	N3s3d::setShader(overlay);
	//	N3s3d::setRasterFillState(false);
	//	// See if it's a OAM or nametable highlight
	//	if (scene->highlight.getHighlightedOAM() >= 0)
	//	{
	//		SceneSprite s = scene->sprites[scene->highlight.getHighlightedOAM()];
	//		Overlay::setColor(0.0f, 1.0f, 0.0f, 1.0f);
	//		Overlay::drawSpriteSquare(s.x, s.y);
	//	}
	//	else if (scene->highlight.getHighlightedNT() >= 0)
	//	{
	//		int xNT = floor(mousePixelX / 8);
	//		int yNT = floor(mousePixelY / 8);
	//		Overlay::setColor(1.0f, 0.0f, 0.0f, 0.5f);
	//		Overlay::drawSpriteSquare(xNT * 8, yNT * 8);
	//	}
	//	N3s3d::setRasterFillState(true);
	//}
	//// Test highlight effect
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
