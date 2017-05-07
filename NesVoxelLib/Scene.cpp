#include "stdafx.h"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Editor.hpp"

#include "N3sConsole.hpp"

XMVECTOR mouseVector;
XMFLOAT3 zIntersect;

Vector3D mousePixelCoordinates;

OrbitCamera mainCamera;

bool mouseCaptured = false;

bool selectionClickedOn = false;
bool draggingSelection = false;
bool movingSelection = false;
int moveX, moveY;
MouseModifier modifier = no_mod;
MouseFunction mouseFunction = no_func;
Vector3D originMousePixelCoordinates;

bool voxelEditing = false;

int sel_top, sel_left, sel_bottom, sel_right;

bool edittingIn3d = false;

bool isSpriteIn2dRect(int top, int left, int bottom, int right, int x, int y)
{
	if ((x >= left && x < right) || (x + 8 >= left && x + 8 < right))
	{
		if ((y >= top && y < bottom) || (y + 8 >= top && y + 8 < bottom))
		{
			return true;
		}
	}
	return false;
}

Scene::Scene()
{
	// Set camera to default position
	mainCamera.SetPosition(0, 0, 0);
	// Clear BG with all "blank" (-1) sprites
	for (int i = 0; i < sceneWidth * sceneHeight; i++)
	{
		bg[i] = { -1, 0, false, false };	
	}
	selection = make_shared<Selection>();
	displaySelection = make_shared<Selection>();
}

Scene::Scene(shared_ptr<PpuSnapshot> snapshot)
{
	// Set camera to default position
	mainCamera.SetPosition(0, 0, 0);
	// Clear BG with all "blank" (-1) sprites
	for (int i = 0; i < sceneWidth * sceneHeight; i++)
	{
		bg[i] = { -1, 0, false, false };
	}
	selection = make_shared<Selection>();
	displaySelection = make_shared<Selection>();
	// Create scene from snapshot
	// Grab all OAM
	for (int i = 0; i < snapshot->sprites.size(); i++)
	{
		OamSprite s = snapshot->sprites[i];
		int tile = snapshot->getTrueOamTile(i);
		int id = N3sApp::virtualPatternTable->getSprite(s.tile)->defaultMesh->id; // lol
		if (id >= 0)
		{
			SceneSprite ss;
			ss.x = s.x;
			ss.y = s.y;
			ss.palette = s.palette;
			ss.meshNum = id;
			ss.mirrorH = s.hFlip;
			ss.mirrorV = s.vFlip;
			sprites.push_back(ss);
		}
	}
	// Grab all NT
	for (int y = 0; y < sceneHeight; y++)
	{
		for (int x = 0; x < sceneWidth; x++)
		{
			NameTableTile t = snapshot->background.getTile(x, y, 0);
			int index = getArrayIndexFromXY(x, y, sceneWidth);
			int tile = snapshot->getTrueNTTile(index);
			shared_ptr<VirtualSprite> s = N3sApp::virtualPatternTable->getSprite(tile);
			int id = s->defaultMesh->id;
			// Add to nametable index
			SceneSprite ss;
			ss.x = 0;
			ss.y = 0;
			ss.palette = t.palette;
			ss.meshNum = id;
			ss.mirrorH = false;
			ss.mirrorV = false;
			bg[index] = ss;
		}
	}
	// Grab palette
	for (int i = 0; i < 8; i++)
		palettes[i] = snapshot->palette;
}

bool Scene::update(bool mouseAvailable)
{
	if (voxelEditing && InputState::functions[voxeleditor_exit].activatedThisFrame)
	{
		voxelEditing = false;
		voxelEditor = nullptr;
	}
	if (!voxelEditing)
	{
		// Update camera position
		if (InputState::keyboardMouse->mouseButtons[right_mouse].state > 0)
		{
			float xRot = InputState::keyboardMouse->mouseDeltaX / 5;
			float yRot = InputState::keyboardMouse->mouseDeltaY / 5;
			mainCamera.AdjustRotation(xRot, yRot, 0.0f);
		}
		if (InputState::keyboardMouse->mouseButtons[middle_mouse].state > 0)
		{
			float xPos = InputState::keyboardMouse->mouseDeltaX / 400;
			float yPos = InputState::keyboardMouse->mouseDeltaY / 400;
			mainCamera.AdjustPosition(-xPos, yPos, 0.0f);
		}
		// Update camera zoom
		mainCamera.adjustZoom((float)InputState::keyboardMouse->calculatedWheelDelta / 10);
		// Update camera math
		mainCamera.Render();
		// Calculate mouse vector and z-intersect
		mouseVector = N3s3d::getMouseVector(&mainCamera, InputState::keyboardMouse->mouseX, InputState::keyboardMouse->mouseY);
		zIntersect = N3s3d::getPlaneIntersection(z_axis, 15, &mainCamera, InputState::keyboardMouse->mouseX, InputState::keyboardMouse->mouseY);
		mousePixelCoordinates = N3s3d::getPixelCoordsFromFloat3(zIntersect);
		return updateMouseActions(mouseAvailable);
	}
	else
	{
		voxelEditor->camera.Render();
		return voxelEditor->update(mouseAvailable);
	}
}

void Scene::render(bool renderBackground, bool renderOAM)
{
	// Enable depth buffer
	N3s3d::setDepthBufferState(true);
	// Update camera math
	if (voxelEditing)
	{
		N3s3d::updateMatricesWithCamera(&voxelEditor->camera);
	}
	else
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
					int i = getArrayIndexFromXY(x, y, sceneWidth);
					if (displaySelection->selectedBackgroundIndices.count(i) > 0 || highlight.getHighlightedNT() == i)
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
			if (displaySelection->selectedSpriteIndices.count(i) > 0 || highlight.getHighlightedOAM() == i)
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
	// Branch based on voxel editing mode
	if (voxelEditing)
	{
		voxelEditor->render();
	}
	else
	{
		// Render highlight selections / mouse hover
		N3s3d::setDepthStencilState(false, false, true);
		N3s3d::setGuiProjection();
		Overlay::setColor(1.0f, 1.0f, 1.0f, 0.3f);
		Overlay::drawRectangle(0, 0, 1920, 1080); // TODO: How do I get actual screen size again..?
		// Draw selection box, if currently dragging selection
		if (draggingSelection)
		{
			N3s3d::setDepthBufferState(false);
			Overlay::setColor(1.0f, 1.0f, 1.0f, 0.3f);
			// Draw differently based on 2D or 3D editing mode
			if (edittingIn3d)
			{
				Overlay::drawRectangle(sel_left, sel_top, sel_right - sel_left, sel_bottom - sel_top);
			}
			else
			{
				// Draw onto z-axis in pixel-space
				N3s3d::updateMatricesWithCamera(&mainCamera);
				Overlay::drawRectangleInScene(sel_left, sel_top, 15, sel_right - sel_left, sel_bottom - sel_top);
			}
		}
		// Render selection boxes around OAM and NT
		N3s3d::setDepthBufferState(false);
		N3s3d::updateMatricesWithCamera(&mainCamera);
		N3s3d::setRasterFillState(false);
		displaySelection->render(&sprites, moveX, moveY);
		N3s3d::setRasterFillState(true);
		
		// Render 3-axis mouse guide
		Overlay::drawAxisLine(zIntersect);

	}
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

void Scene::changeSelectionPalette(int p)
{
	if (p <= 3)
	{
		for each (auto i in selection->selectedBackgroundIndices)
		{
			bg[i].palette = p;
		}
	}
	else
	{
		for each (auto i in selection->selectedSpriteIndices)
		{
			sprites[i].palette = p;
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

void Scene::updateHighlight2d(Vector3D mouse, bool highlightOAM, bool highlightNametable)
{
	// Clear previous highlight
	highlight.clear();
	// See if any OAM sprites intersect selection
	if (highlightOAM)
	{
		for (int i = 0; i < sprites.size(); i++)
		{
			SceneSprite s = sprites[i];
			if (mouse.x >= s.x && mouse.x < s.x + 8 && mouse.y >= s.y && mouse.y < s.y + 8)
				highlight.highlightedSpriteIndices.push_back(i);
		}
	}
	// See if any part of the background intersects selection
	if (highlightNametable)
	{
		if (mouse.x >= 0 && mouse.x < scenePixelWidth && mouse.y >= 0 && mouse.y < scenePixelHeight)
			highlight.highlightedBackgroundIndex = floor(mouse.y / 8) * 64 + floor(mouse.x / 8);
	}
	// Set the index
	if (highlight.highlightedBackgroundIndex >= 0 || highlight.highlightedSpriteIndices.size() > 0)
		highlight.selectedIndex = 0;
}

bool Scene::updateMouseActions(bool mouseAvailable)
{
	// If mouse has moved and is available, calculate highlighted items
	if (InputState::keyboardMouse->hasMouseMoved())
		updateHighlight2d(mousePixelCoordinates, true, true);
	// Check left mouse
	if (mouseAvailable || mouseCaptured)
	{
		MouseState state = InputState::keyboardMouse->mouseButtons[left_mouse].state;
		if (state > 0 && mouseAvailable)
			mouseCaptured = true;
		else
			mouseCaptured = false;
		int mouseX = InputState::keyboardMouse->mouseX;
		int mouseY = InputState::keyboardMouse->mouseY;
		if (mouseCaptured)
		{
			// See if this is a new click
			if (state == clicked)
			{
				// See if OAM or NT is highlighted at all
				if (highlight.anythingHighlighted())
				{
					// See if the current selection is highlighted
					if(selection->selectedSpriteIndices.count(highlight.getHighlightedOAM()) > 0 ||
					   selection->selectedBackgroundIndices.count(highlight.getHighlightedNT()) > 0)
						selectionClickedOn = true;
				}
				// Capture mod key
				if (InputState::functions[selection_add].active && InputState::functions[selection_remove].active)
					modifier = mod_intersect;
				else if (InputState::functions[selection_add].active)
					modifier = mod_add;
				else if (InputState::functions[selection_remove].active)
					modifier = mod_remove;
				else if (InputState::functions[selection_copy].active)
					modifier = mod_copy;
				else
				{
					modifier = no_mod;
				}
				// Capture Z-intersect at click
				originMousePixelCoordinates = mousePixelCoordinates; //N3s3d::getPixelCoordsFromFloat3(zIntersect);
			}
			else if (state == pressed)
			{
				// Check for highlight and add to selection or enter voxel editor
				if (highlight.getHighlightedOAM() >= 0)
				{
					if (modifier == no_mod && selection->selectedSpriteIndices.size() == 1)
					{
						auto selected = selection->selectedSpriteIndices.begin();
						int s = *selected;
						if (highlight.getHighlightedOAM() == s)
						{
							// Switch to editing that mesh
							N3sConsole::writeLine("SWITCHED TO EDITOR!");
							int meshNum = sprites[s].meshNum;
							shared_ptr<SpriteMesh> mesh = N3sApp::gameData->meshes[meshNum];
							voxelEditor = make_shared<VoxelEditor>(mesh, sprites[s].x, sprites[s].y, mainCamera);
							voxelEditing = true;
						}
						else
						{
							selection->clear();
							selection->selectedSpriteIndices.insert(highlight.getHighlightedOAM());
						}
					}
					else if (modifier == no_mod)
					{
						selection->clear();
						selection->selectedSpriteIndices.insert(highlight.getHighlightedOAM());
					}
					else if (modifier == mod_add)
						selection->selectedSpriteIndices.insert(highlight.getHighlightedOAM());
					else if (modifier == mod_remove)
						selection->selectedSpriteIndices.erase(highlight.getHighlightedOAM());
				}
				else if (highlight.getHighlightedNT() >= 0)
				{
					if (modifier == no_mod)
					{
						selection->clear();
						selection->selectedBackgroundIndices.insert(highlight.getHighlightedNT());
					}
					else if (modifier == mod_add)
						selection->selectedBackgroundIndices.insert(highlight.getHighlightedNT());
					else if (modifier == mod_remove)
						selection->selectedBackgroundIndices.erase(highlight.getHighlightedNT());
				}
			}
			else if (state == dragging)
			{
				// If we highlighted any part of our selection, then mouse dragging should move the whole thing
				if (selectionClickedOn)
				{
					movingSelection = true;
					// Set the movement amount
					moveX = mousePixelCoordinates.x - originMousePixelCoordinates.x;
					moveY = mousePixelCoordinates.y - originMousePixelCoordinates.y;
				}
				// If a modifier was held down, or no part of selection was highlighted, start drag selection
				if (!selectionClickedOn)
				{
					if(modifier == no_mod)
						selection->clear();
					draggingSelection = true;
					// Calculate the new selection area
					int x, y, startX, startY;
					// Branch based on 2D or 3D editing mode
					if (edittingIn3d)
					{
						// Get X&Y or beginning and end of drag in screen-space
						x = mouseX;
						y = mouseY;
						startX = InputState::keyboardMouse->mouseButtons[left_mouse].actionXStart;
						startY = InputState::keyboardMouse->mouseButtons[left_mouse].actionYStart;
					}
					else
					{
						// Get X&Y or beginning and end of drag in game-space
						x = mousePixelCoordinates.x;
						y = mousePixelCoordinates.y;
						startX = originMousePixelCoordinates.x;
						startY = originMousePixelCoordinates.y;
					}
					// Set sides of selection
					if (x > startX)
					{
						sel_right = x;
						sel_left = startX;
					}
					else
					{
						sel_right = startX;
						sel_left = x;
					}
					if (y > startY)
					{
						sel_top = startY;
						sel_bottom = y;
					}
					else
					{
						sel_top = y;
						sel_bottom = startY;
					}
					// Get all items current in the selection box in their own selection
					shared_ptr<Selection> tempSelection = make_shared<Selection>();
					if (true) // TODO add actual options
					{
						for(int i = 0; i < sprites.size(); i++)
						{
							SceneSprite s = sprites[i];
							if (isSpriteIn2dRect(sel_top, sel_left, sel_bottom, sel_right, sprites[i].x, sprites[i].y))
								tempSelection->selectedSpriteIndices.insert(i);
						}
					}
					if (true)
					{
						for (y = 0; y < sceneHeight; y++)
							for (x = 0; x < sceneWidth; x++)
								if (isSpriteIn2dRect(sel_top, sel_left, sel_bottom, sel_right, x * 8, y * 8))
								{
									int i = getArrayIndexFromXY(x, y, sceneWidth);
									// Check that we actually have something there, and the sprite isn't empty (no mesh)
									if(bg[i].meshNum >= 0 && N3sApp::gameData->meshes[bg[i].meshNum]->meshExists)
										tempSelection->selectedBackgroundIndices.insert(i);
								}
					}
					// Set new display selection to be some combination of the two, depending on modifier key
					switch (modifier)
					{
					case mod_add:
						displaySelection = Selection::getUnion(selection, tempSelection);
						break;
					case mod_remove:
						displaySelection = Selection::getSubtraction(selection, tempSelection);
						break;
					case mod_intersect:
						displaySelection = Selection::getIntersection(selection, tempSelection);
						break;
					default:	// No modifier
						displaySelection = tempSelection;
						break;
					}
				}
			}
		}
	}
	if(!mouseAvailable || !mouseCaptured)
	{
		// Set display selection to the new, real selection if we just stopped dragging
		if(draggingSelection)
			selection = displaySelection;
		// Move the items, if we just finished moving
		if (movingSelection)
			if (modifier == mod_copy)
				moveSelection(true);
			else
				moveSelection(false);
		mouseFunction = no_func;
		modifier = no_mod;
		draggingSelection = false;
		selectionClickedOn = false;
		movingSelection = false;
		moveX = 0;
		moveY = 0;
	}
	// The displayed selection is just the current selection if we're not dragging
	// (This might be redundant if we've just finished dragging, oh well)
	if (!draggingSelection)
		displaySelection = selection;
	return true;
}

void Scene::moveSelection(bool copy) // TODO add copy modifier parameter
{
	// Move or copy OAM sprites
	if (copy)
	{
		// Track indices of new OAM sprites, which will be selected after operation
		unordered_set<int> newOAMIndices;
		for each(int i in selection->selectedSpriteIndices)
		{
			int newIndex = sprites.size();	// Grab new index
			SceneSprite newSprite = sprites[i];
			newSprite.x += moveX;
			newSprite.y += moveY;
			sprites.push_back(newSprite);
			newOAMIndices.insert(newIndex);
		}
		selection->selectedSpriteIndices = newOAMIndices;
	}
	else
	{
		for each(int i in selection->selectedSpriteIndices)
		{
			sprites[i].x += moveX;
			sprites[i].y += moveY;
		}
	}

	// Calculate how much the nametable sprites have moved
	int ntX = roundDownPosOrNeg(moveX / 8);
	int ntY = roundDownPosOrNeg(moveY / 8);
	// Cache old values
	unordered_map<int, SceneSprite> cache;
	for each(int i in selection->selectedBackgroundIndices)
	{
		cache[i] = bg[i];
	}
	// Clear old values, if this isn't a duplication
	for (auto kv : cache)
	{
		if (!copy)
		{
			bg[kv.first] = { -1, 0, false, false };
		}
		selection->selectedBackgroundIndices.erase(kv.first);
	}
	// Overwrite NT at new positions
	for (auto kv : cache)
	{
		Vector2D oldXY = unwrapArrayIndex(kv.first, sceneWidth);
		int newIndex = getArrayIndexFromXY(oldXY.x + ntX, oldXY.y + ntY, sceneWidth);
		// Make sure this new index is within bounds
		if (newIndex >= 0 && newIndex < bgSize)
		{
			bg[newIndex] = kv.second;
			selection->selectedBackgroundIndices.insert(newIndex);
		}
	}
}

Vector2D Scene::getCoordinatesFromZIntersection(XMFLOAT3 zIntersect)
{
	Vector2D mouse;
	float xAtIntersect = zIntersect.x;
	float yAtIntersect = zIntersect.y;
	// Normalize top-left of all screens to 0,0
	xAtIntersect += 1.0f;
	yAtIntersect -= 1.0f;
	// Divide each by full size of scene
	xAtIntersect /= sceneDXWidth;
	yAtIntersect /= sceneDXHeight;
	// Get "pixel" position of X
	mouse.x = floor(scenePixelWidth * xAtIntersect);
	// Get Y, but flip it first (since negative = positive in NES screenspace)
	mouse.y = floor(scenePixelHeight * (yAtIntersect * -1));
	return mouse;
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

shared_ptr<Selection> Selection::getUnion(shared_ptr<Selection> a, shared_ptr<Selection> b)
{
	shared_ptr<Selection> temp = make_shared<Selection>();
	for each(int i in a->selectedSpriteIndices)
		temp->selectedSpriteIndices.insert(i);
	for each(int i in b->selectedSpriteIndices)
		temp->selectedSpriteIndices.insert(i);
	for each(int i in a->selectedBackgroundIndices)
		temp->selectedBackgroundIndices.insert(i);
	for each(int i in b->selectedBackgroundIndices)
		temp->selectedBackgroundIndices.insert(i);
	return temp;
}

shared_ptr<Selection> Selection::getSubtraction(shared_ptr<Selection> first, shared_ptr<Selection> second)
{
	shared_ptr<Selection> temp = make_shared<Selection>();
	// Get all selections for first sets
	temp->selectedSpriteIndices = first->selectedSpriteIndices;
	temp->selectedBackgroundIndices = first->selectedBackgroundIndices;
	// Remove any that are also in the second sets
	for each(int i in second->selectedSpriteIndices)
		temp->selectedSpriteIndices.erase(i);
	for each(int i in second->selectedBackgroundIndices)
		temp->selectedBackgroundIndices.erase(i);
	return temp;
}

shared_ptr<Selection> Selection::getIntersection(shared_ptr<Selection> a, shared_ptr<Selection> b)
{
	shared_ptr<Selection> temp = make_shared<Selection>();
	// Add sprite and nt indices if both selections have them
	for each(int i in a->selectedSpriteIndices)
	{
		if(b->selectedSpriteIndices.count(i) > 0)
			temp->selectedSpriteIndices.insert(i);
	}
	for each(int i in b->selectedBackgroundIndices)
	{
		if (b->selectedBackgroundIndices.count(i) > 0)
			temp->selectedBackgroundIndices.insert(i);
	}
	return temp;
}

void Selection::render(vector<SceneSprite> * sprites, int moveX, int moveY)
{
	// Render all OAM highlights
	Overlay::setColor(0.0f, 1.0f, 0.0f, 1.0f);
	for each(int i in selectedSpriteIndices)
	{
		SceneSprite s = sprites->at(i);
		Overlay::drawSpriteSquare(s.x + moveX, s.y + moveY);
	}
	// Render all NT highlights
	Overlay::setColor(1.0f, 0.0f, 0.0f, 1.0f);
	for each(int i in selectedBackgroundIndices)
	{
		int ntX = roundDownPosOrNeg(moveX / 8);
		int ntY = roundDownPosOrNeg(moveY / 8);
		Vector2D pos = unwrapArrayIndex(i, sceneWidth);
		Overlay::drawSpriteSquare((pos.x + ntX) * 8, (pos.y + ntY) * 8);
	}
}