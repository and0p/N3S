#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"

shared_ptr<Scene> scenes[sceneCount];
shared_ptr<Scene> tempScene;
shared_ptr<Scene> sheetScene;

shared_ptr<Scene> activeScene;

int Editor::selectedScene = 0;
int selectedSprite;
bool editing;

SceneSelector sceneSelector;
PaletteSelector paletteSelector;

bool Editor::mouseAvailable = true;
N3sPalette Editor::copiedPalette;

bool chrSheetsCreated = false;

void Editor::init()
{
	for (int i = 0; i < sceneCount; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
}

void Editor::loadJSON(json j)
{
	// Load all scenes
	for (int i = 0; i < 16; i++)
	{
		// Make sure scene is specified in JSON
		if (j.size() > i)
		{
			scenes[i] = make_shared<Scene>((json)j[i]);
		}
		else
		{
			// Add empty scene
			scenes[i] = make_shared<Scene>();
		}
	}
}

json Editor::getJSON()
{
	json j;
	for (int i = 0; i < 16; i++)
	{
		j[i] = scenes[i]->getJSON();
	}
	return j;
}

void Editor::update()
{
	if (activeScene == nullptr)
		activeScene = scenes[0];
	mouseAvailable = sceneSelector.update(mouseAvailable);
	mouseAvailable = paletteSelector.update(mouseAvailable, activeScene, activeScene->voxelEditor);
	shared_ptr<VoxelEditor> editor = activeScene->voxelEditor;
	if (editor != nullptr)
	{
		mouseAvailable = MeshInfo::update(mouseAvailable, editor->mesh);
		mouseAvailable = VoxelEditorInfo::update(mouseAvailable, editor);
	}
	else
	{
		MeshInfo::clear();
		VoxelEditorInfo::clear();
		// See if copy buttons were pressed
		if (InputState::functions[editor_copy].activatedThisFrame)
			copiedSprites = activeScene->copySelection();
		else if (InputState::functions[editor_paste].activatedThisFrame)
			activeScene->pasteSelection(copiedSprites);
		// Check copy/paste palette keys
		if (InputState::functions[palette_copy].activatedThisFrame)
			copiedPalette = *activeScene->getSelectedPalette();
		if (InputState::functions[palette_paste].activatedThisFrame)
			activeScene->palettes[activeScene->selectedPalette] = copiedPalette;
	}

	// See if close button was pressed, when voxel editing
	if (editor != nullptr && VoxelEditorInfo::e == nullptr)
	{
		editor = nullptr;
		activeScene->voxelEditor = nullptr;
		MeshInfo::clear();
		VoxelEditorInfo::clear();
	}

	activeScene->update(mouseAvailable);
}

void Editor::parseInputForScene(bool mouseAvailable)
{
	
}

void Editor::render()
{
	shared_ptr<Scene> scene = activeScene;

	// Render the scene
	scene->render(true, true);
	// Render overlays
	scene->renderOverlays(true, true);
	// Render GUI
	N3s3d::setDepthBufferState(false);
	N3s3d::setGuiProjection();
	sceneSelector.render();
	paletteSelector.render(scene, activeScene->voxelEditor);
	MeshInfo::render();
	VoxelEditorInfo::render();
}

void Editor::setScene(int s)
{
	if (s >= 0 && s < 17)
	{
		selectedScene = s;
		if (s < 16)
			activeScene = scenes[s];
		else if (s = 17)
			activeScene = tempScene;
	}
}

Hue Editor::getBackgroundColor()
{
	return activeScene->getSelectedPalette()->getBackgroundColor();
}

void Editor::updateTempScene(shared_ptr<PpuSnapshot> snapshot)
{
	// Update VRAM scene
	if (activeScene == tempScene)
	{
		// Also make active scene reference new one, if it was being viewed last time editor was open
		tempScene.reset(new Scene(snapshot));
		activeScene = tempScene;
	}
	else
	{
		tempScene.reset(new Scene(snapshot));
	}
}

void Editor::createCHRSheet(int pageNumber)
{
	if (!chrSheetsCreated)
	{
		sheetScene.reset(new Scene());
		// Generate helpful palettes for typical sprites
		N3sPalette p;
		p.backgroundColorIndex = 0; // dark grey
		p.colorIndices[0] = 1;
		p.colorIndices[1] = 17;
		p.colorIndices[2] = 49;
		p.colorIndices[3] = 2;
		p.colorIndices[4] = 18;
		p.colorIndices[5] = 50;
		p.colorIndices[6] = 3;
		p.colorIndices[7] = 19;
		p.colorIndices[8] = 51;
		p.colorIndices[9] = 4;
		p.colorIndices[10] = 20;
		p.colorIndices[11] = 52;
		p.colorIndices[12] = 5;
		p.colorIndices[13] = 21;
		p.colorIndices[14] = 53;
		p.colorIndices[15] = 6;
		p.colorIndices[16] = 22;
		p.colorIndices[17] = 54;
		p.colorIndices[18] = 7;
		p.colorIndices[19] = 23;
		p.colorIndices[20] = 55;
		p.colorIndices[21] = 10;
		p.colorIndices[22] = 26;
		p.colorIndices[23] = 56;
		sheetScene->palettes[0] = p;
	}
	sheetScene->sprites.clear();
	sheetScene->selection->clear();
	// Add 256 sprites to scene
	int startingSprite = pageNumber * 256;
	selectedScene = -1;
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			SceneSprite s;
			s.x = x * 16;
			s.y = y * 16;
			s.palette = 0;
			// Make sure mesh exists
			int index = startingSprite + getArrayIndexFromXY(x, y, 8);
			if (N3sApp::gameData->meshes.size() > index)
			{
				shared_ptr<SpriteMesh> ref = N3sApp::gameData->meshes[index];
				s.mesh = ref;
				sheetScene->addSprite(s);
			}
		}
	}
	activeScene = sheetScene;
	chrSheetsCreated = true;
}
