#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"

shared_ptr<Scene> scenes[sceneCount];

int selectedScene = 0;
int selectedSprite;
bool editing;

SceneSelector sceneSelector;
PaletteSelector paletteSelector;

bool Editor::mouseAvailable;

void Editor::init()
{
	mouseAvailable = true;
	for (int i = 0; i < sceneCount; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
	scenes[0]->setBackgroundSprite(15, 15, { nullptr,3,1,0,0,false,false });
	scenes[0]->addOAMSprite({ nullptr, 3, 0, 0, 0, false, false });
	scenes[0]->addOAMSprite({ nullptr, 3, 0, 200, 171, false, false });
	scenes[0]->addOAMSprite({ nullptr, 3, 0, 133, 117, false, false });
	scenes[0]->addOAMSprite({ nullptr, 3, 0, 32, 32, false, false });
}

void Editor::update()
{
	mouseAvailable = sceneSelector.update(mouseAvailable);
	mouseAvailable = paletteSelector.update(mouseAvailable, scenes[selectedScene], scenes[selectedScene]->voxelEditor);
	shared_ptr<VoxelEditor> editor = scenes[selectedScene]->voxelEditor;
	if (editor != nullptr)
	{
		mouseAvailable = MeshInfo::update(mouseAvailable, editor->mesh);
		mouseAvailable = VoxelEditorInfo::update(mouseAvailable, editor);
	}
	else
	{
		MeshInfo::clear();
		VoxelEditorInfo::clear();
	}

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
	paletteSelector.render(scene, nullptr);
	MeshInfo::render();
	VoxelEditorInfo::render();
}

void Editor::setScene(int s)
{
	if(s >= 0 && s < sceneCount)
		selectedScene = s;
}

Hue Editor::getBackgroundColor()
{
	return scenes[selectedScene]->getSelectedPalette()->getBackgroundColor();
}

void Editor::updateTempScene(shared_ptr<PpuSnapshot> snapshot)
{
	scenes[15].reset(new Scene(snapshot));
}