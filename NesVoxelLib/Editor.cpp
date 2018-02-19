#include "stdafx.h"
#include "Editor.hpp"
#include "Gui.hpp"
#include "N3sApp.hpp"
#include "Input.hpp"

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

shared_ptr<OrbitCamera> camera = make_shared<OrbitCamera>();

void Editor::init()
{
	camera->SetPosition(0.0f, 0.0f, pixelSizeW * 16);
	for (int i = 0; i < sceneCount; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
}

void Editor::loadJSON(json j)
{
	camera->SetPosition(0.0f, 0.0f, pixelSizeW * 16);
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
	shared_ptr<VoxelEditor> voxelEditor = activeScene->voxelEditor;
	if (voxelEditor != nullptr)
	{
		mouseAvailable = MeshInfo::update(mouseAvailable, voxelEditor->mesh);
		mouseAvailable = VoxelEditorInfo::update(mouseAvailable, voxelEditor);
		// See if user wants to select adjacent sprite
		int selectedSprite = -1;
		if (InputState::functions[voxeleditor_select_up]->activatedThisFrame)
			selectedSprite = activeScene->findNearestSprite(activeScene->spriteBeingEdited, select_up);
		else if (InputState::functions[voxeleditor_select_down]->activatedThisFrame)
			selectedSprite = activeScene->findNearestSprite(activeScene->spriteBeingEdited, select_down);
		else if (InputState::functions[voxeleditor_select_left]->activatedThisFrame)
			selectedSprite = activeScene->findNearestSprite(activeScene->spriteBeingEdited, select_left);
		else if (InputState::functions[voxeleditor_select_right]->activatedThisFrame)
			selectedSprite = activeScene->findNearestSprite(activeScene->spriteBeingEdited, select_right);
		// If so, and sprite found, change sprite in voxel editor + selection in scene
		if (selectedSprite >= 0)
		{
			voxelEditor->changeSprite(&activeScene->sprites[selectedSprite]);
			activeScene->selection->clear();
			activeScene->selection->selectedIndices.insert(selectedSprite);
			activeScene->spriteBeingEdited = selectedSprite;
		}
	}
	else
	{
		MeshInfo::clear();
		VoxelEditorInfo::clear();
		// See if copy buttons were pressed
		if (InputState::functions[editor_copy]->activatedThisFrame)
			copiedSprites = activeScene->copySelection();
		else if (InputState::functions[editor_paste]->activatedThisFrame)
			activeScene->pasteSelection(copiedSprites);
		// Check copy/paste palette keys
		if (InputState::functions[palette_copy]->activatedThisFrame)
			copiedPalette = *activeScene->getSelectedPalette();
		if (InputState::functions[palette_paste]->activatedThisFrame)
			activeScene->palettes[activeScene->selectedPalette] = copiedPalette;
		if (activeScene->selection->selectedIndices.size() > 0 &&
		   (InputState::functions[editor_flip_sprite_h]->activatedThisFrame || 
			InputState::functions[editor_flip_sprite_v]->activatedThisFrame))
		{
			bool hFlip = InputState::functions[editor_flip_sprite_h]->activatedThisFrame;
			bool vFlip = InputState::functions[editor_flip_sprite_v]->activatedThisFrame;
			for each(int i in activeScene->selection->selectedIndices)
			{
				activeScene->sprites[i].flip(hFlip, vFlip);
			}
		}
		// Move camera
		float camXMove = (InputState::functions[cam_move_right]->value * 0.01f) - (InputState::functions[cam_move_left]->value * 0.01f);
		float camYMove = (InputState::functions[cam_move_up]->value * 0.01f) - (InputState::functions[cam_move_down]->value * 0.01f);
		camXMove *= camera->zoom;
		camYMove *= camera->zoom;
		camera->AdjustPosition(camXMove, camYMove, 0.0f);
	}

	// See if close button was pressed, when voxel editing
	if (voxelEditor != nullptr && VoxelEditorInfo::e == nullptr)
	{
		voxelEditor = nullptr;
		activeScene->voxelEditor = nullptr;
		MeshInfo::clear();
		VoxelEditorInfo::clear();
	}

	// Update editor camera
	// Update camera position
	if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
	{
		float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
		float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
		camera->AdjustRotation(xRot, yRot, 0.0f);
	}
	if (InputState::keyboardMouse->mouseButtons[middle_mouse].state > 0)
	{
		float xPos = InputState::keyboardMouse->mouseDeltaX / 400;
		float yPos = InputState::keyboardMouse->mouseDeltaY / 400;
		camera->AdjustPosition(-xPos, yPos, 0.0f);
	}
	// Update camera zoom
	if(activeScene->voxelEditor == nullptr)
		camera->adjustZoom((float)InputState::keyboardMouse->calculatedWheelDelta / 10);
	// Update camera math
	camera->Render();

	activeScene->update(mouseAvailable, camera);
}

void Editor::parseInputForScene(bool mouseAvailable)
{
	
}

void Editor::render()
{
	shared_ptr<Scene> scene = activeScene;

	// Render overlays
	scene->renderOverlays(true, false);
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
		chrSheetsCreated = true;
	}
	sheetScene->sprites.clear();
	sheetScene->selection->clear();
	sheetScene->highlight.clear();
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
			s.mirrorH = false;
			s.mirrorV = false;
			s.palette = 0;	// anything more there
			// Make sure mesh exists
			int index = startingSprite + getArrayIndexFromXY(x, y, 16);
			if (N3sApp::gameData->meshes.size() > index)
			{
				shared_ptr<SpriteMesh> ref = N3sApp::gameData->meshes[index];
				s.mesh = ref;
				sheetScene->addSprite(s);
			}
		}
	}
	activeScene = sheetScene;
}

shared_ptr<Scene> Editor::getSelectedScene()
{
	return activeScene;
}

shared_ptr<Camera> Editor::getCamera()
{
	if (activeScene->voxelEditor == nullptr)
		return camera;
	else
		return activeScene->voxelEditor->camera;
}
