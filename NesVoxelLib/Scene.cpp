#include "stdafx.h"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Editor.hpp"
#include "N3sConsole.hpp"

XMVECTOR mouseVector;
XMFLOAT3 zIntersect;

Vector2D mousePixelCoordinates;

OrbitCamera mainCamera;

bool mouseCaptured = false;

bool selectionClickedOn = false;
bool draggingSelection = false;
bool movingSelection = false;
int moveX, moveY;
MouseModifier modifier = no_mod;
MouseFunction mouseFunction = no_func;
Vector2D originMousePixelCoordinates;
Vector2D originMousePixelCoordinatesModified;

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
		//bg[i] = { nullptr, 0, false, false };
	}
	selection = make_shared<Selection>();
	displaySelection = make_shared<Selection>();
	// Create scene from snapshot
	// Grab all OAM
	for (int i = 0; i < snapshot->sprites.size(); i++)
	{
		OamSprite s = snapshot->sprites[i];
		int tile = snapshot->getTrueOamTile(i);
		shared_ptr<SpriteMesh> spriteMesh = N3sApp::virtualPatternTable->getSprite(tile)->defaultMesh; // TODO get virtually defined mesh?
		if (spriteMesh->meshExists >= 0 && s.x >= 0 && s.x < 256 && s.y >= 0 && s.y < 240)
		{
			SceneSprite ss;
			ss.mesh = spriteMesh;
			ss.x = s.x;
			ss.y = s.y;
			ss.palette = s.palette;
			ss.mirrorH = s.hFlip;
			ss.mirrorV = s.vFlip;
			// Mirror and change position differently based on 16x8 mode
			if (snapshot->ctrl.spriteSize16x8)
			{
				if (s.vFlip)
					ss.y += 8;
			}
			sprites.push_back(ss);
		}
		if (snapshot->ctrl.spriteSize16x8)
		{
			// Get 16x8 sprites as well
			tile++;
			spriteMesh = N3sApp::virtualPatternTable->getSprite(tile)->defaultMesh;
			if (spriteMesh->meshExists >= 0)
			{
				SceneSprite ss;
				ss.mesh = spriteMesh;
				ss.x = s.x;
				ss.y = s.y + 8;
				ss.palette = s.palette;
				ss.mirrorH = s.hFlip;
				ss.mirrorV = s.vFlip;
				if (s.vFlip)
					ss.y -= 8;
				sprites.push_back(ss);
			}
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
			shared_ptr<SpriteMesh> spriteMesh = s->defaultMesh;
			if (spriteMesh->meshExists)
			{
				SceneSprite ss;
				ss.mesh = spriteMesh;
				ss.x = x * 8;
				ss.y = y * 8;
				ss.palette = t.palette;
				ss.mirrorH = false;
				ss.mirrorV = false;
				sprites.push_back(ss);
			}

		}
	}
	// Grab palette
	for (int i = 0; i < 8; i++)
		palettes[i] = snapshot->palette;
}

bool Scene::update(bool mouseAvailable)
{
	if (voxelEditor != nullptr && InputState::functions[voxeleditor_exit].activatedThisFrame)
	{
		voxelEditor = nullptr;
	}
	if (voxelEditor == nullptr)
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
		Vector3D zIntersectPixels = N3s3d::getPixelCoordsFromFloat3(zIntersect);
		mousePixelCoordinates.x = zIntersectPixels.x;
		mousePixelCoordinates.y = zIntersectPixels.y;
		// See if the modifier is being pressed to show additional guides
		showGuides = InputState::functions[editor_alt].active;
		// If we have selection and user has hit ESC, unselect all
		if (InputState::functions[selection_deselect].activatedThisFrame)
			selection->selectedIndices.clear();
		// If we have a selection and user has pressed delete, delete the selected sprites
		if (InputState::functions[selection_delete].activatedThisFrame)
			deleteSelection();
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
	if (voxelEditor != nullptr)
	{
		N3s3d::updateMatricesWithCamera(&voxelEditor->camera);
	}
	else
		N3s3d::updateMatricesWithCamera(&mainCamera);
	// Update palette in video card
	palettes[selectedPalette].updateShaderPalette();
	// Render sprites
	for (int i = 0; i < sprites.size(); i++)
	{
		SceneSprite s = sprites[i];
		// See if sprite has any mesh
		if (s.mesh != nullptr)
		{
			// See if sprite is highlighted and write to stencil buffer if so
			if (displaySelection->selectedIndices.count(i) > 0 || highlight.highlightedIndices.count(i) > 0)
			{
				N3s3d::setDepthStencilState(true, true, false);
				s.mesh->render(s.x, s.y, s.palette, s.mirrorH, s.mirrorV, { 0, 0, 0, 0 });
				N3s3d::setDepthStencilState(true, false, false);
			}
			else
			{
				s.mesh->render(s.x, s.y, s.palette, s.mirrorH, s.mirrorV, { 0, 0, 0, 0 });
			}
		}
	}
	// Branch based on voxel editing mode
	if (voxelEditor != nullptr)
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
		if(showGuides)
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
	if (drawBackgroundGrid && showGuides)
	{
		Overlay::setColor(1.0f, 0.0f, 0.0f, 0.1f);
		Overlay::drawNametableGrid(0, 0);
		Overlay::drawNametableGrid(32, 0);
		Overlay::drawNametableGrid(0, 30);
		Overlay::drawNametableGrid(32, 30);
	}
	// Draw OAM highlights, if enabled
	if (drawOamHighlights && showGuides)
	{
		Overlay::setColor(0.0f, 1.0f, 0.0f, 1.0f);
		for each(SceneSprite s in sprites)
		{
			Overlay::drawSpriteSquare(s.x, s.y);
		}
	}
	N3s3d::setRasterFillState(true);
}

void Scene::changeSelectionPalette(int p)
{
	for each (auto i in selection->selectedIndices)
	{
		sprites[i].palette = p;
	}
}

void Scene::addSprite(SceneSprite s)
{
	sprites.push_back(s);
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

void Scene::updateHighlight2d(Vector2D mouse, bool highlightOAM, bool highlightNametable)
{
	// Clear previous highlight
	highlight.clear();
	// See if any sprites intersect selection
	for (int i = 0; i < sprites.size(); i++)
	{
		SceneSprite s = sprites[i];
		if (mouse.x >= s.x && mouse.x < s.x + 8 && mouse.y >= s.y && mouse.y < s.y + 8)
			highlight.highlightedIndices.insert(i);
	}
	if (highlight.highlightedIndices.size() > 0)
		highlight.anythingHighlighted = true;
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
				if (highlight.anythingHighlighted)
				{
					selectionClickedOn = false;
					// See if the current selection is highlighted
					for each(int i in selection->selectedIndices)
					{
						if(i == highlight.getHighlight())
							selectionClickedOn = true;
					}
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
				if (highlight.getHighlight() >= 0)
				{
					if (modifier == no_mod && selection->selectedIndices.size() == 1)
					{
						auto selected = selection->selectedIndices.begin();
						int s = *selected;
						if (highlight.getHighlight() == s)
						{
							// Switch to editing that mesh
							N3sConsole::writeLine("SWITCHED TO EDITOR!");
							voxelEditor = make_shared<VoxelEditor>(sprites[s].mesh, sprites[s].x, sprites[s].y, sprites[s].mirrorH, sprites[s].mirrorV, mainCamera);
						}
						else
						{
							selection->clear();
							selection->selectedIndices.insert(highlight.getHighlight());
						}
					}
					else if (modifier == no_mod)
					{
						selection->clear();
						selection->selectedIndices.insert(highlight.getHighlight());
					}
					else if (modifier == mod_add)
						selection->selectedIndices.insert(highlight.getHighlight());
					else if (modifier == mod_remove)
						selection->selectedIndices.erase(highlight.getHighlight());
				}
			}
			else if (state == dragging)
			{
				// Get the scene X/Y of the dragging destination, possibly modified (snapped)
				dragStart = originMousePixelCoordinates;
				dragDestination = mousePixelCoordinates;
				// If we highlighted any part of our selection, then mouse dragging should move the whole thing
				if (selectionClickedOn)
				{
					movingSelection = true;
					// Get top-left sprite in selection, and snap based off that if needed
					Vector2D topLeftSpriteXY = getTopLeftSpriteInSelection(displaySelection);
					// Use mod keys to snap to nametable

					if (InputState::functions[selection_add].active && InputState::functions[selection_remove].active)
					{
						// Get top-left sprites offset from nearest snap point
						Vector2D offset = { topLeftSpriteXY.x % 8, topLeftSpriteXY.y % 8 };
						// Grab drag destination X and Y but only in multiples of 8
						Vector2D diffSnapped = Vector2D::diff(dragStart, dragDestination);
						diffSnapped.snap();
						// Subtract this from current dragging destination
						dragDestination.sub(offset);
						dragDestination.add(diffSnapped);
						// Set the movement amount
						moveX = -offset.x + diffSnapped.x;
						moveY = -offset.y + diffSnapped.y;
					}
					 else if (InputState::functions[selection_add].active)
					{
						// Snap to relative 8
						dragDestination.snapRelative(dragStart);
						// Set the movement amount
						moveX = dragDestination.x - dragStart.x;
						moveY = dragDestination.y - dragStart.y;
					}
					else
					{
						// Set the movement amount
						moveX = dragDestination.x - dragStart.x;
						moveY = dragDestination.y - dragStart.y;
					}
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
								tempSelection->selectedIndices.insert(i);
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
		unordered_set<int> newIndices;
		for each(int i in selection->selectedIndices)
		{
			int newIndex = sprites.size();	// Grab new index
			SceneSprite newSprite = sprites[i];
			newSprite.x += moveX;
			newSprite.y += moveY;
			sprites.push_back(newSprite);
			newIndices.insert(newIndex);
		}
		selection->selectedIndices = newIndices;
	}
	else
	{
		for each(int i in selection->selectedIndices)
		{
			sprites[i].x += moveX;
			sprites[i].y += moveY;
		}
	}
}

shared_ptr<SpriteMesh> Scene::getSelectedMesh()
{
		return nullptr;
}

void Scene::deleteSelection()
{
	// TODO super inefficient!
	// languages that don't have basic, simple collection functions are real cool!~
	vector<SceneSprite> newList;
	for (int i = 0; i < sprites.size(); i++)
	{
		if (selection->selectedIndices.count(i) < 1)
			newList.push_back(sprites[i]);
	}
	sprites = newList;
	selection->selectedIndices.clear();
	displaySelection = selection;
}

Vector3F Scene::getCoordinatesFromZIntersection(XMFLOAT3 zIntersect)
{
	Vector3F mouse;
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

int Highlight::getHighlight()
{
	if (highlightedIndices.size() > 0)
		return *highlightedIndices.begin();
	else
		return -1;
}

void Highlight::clear()
{
	highlightedIndices.clear();
	anythingHighlighted = false;
}

void Selection::clear()
{
	selectedIndices.clear();
}

shared_ptr<Selection> Selection::getUnion(shared_ptr<Selection> a, shared_ptr<Selection> b)
{
	shared_ptr<Selection> temp = make_shared<Selection>();
	for each(int i in a->selectedIndices)
		temp->selectedIndices.insert(i);
	for each(int i in b->selectedIndices)
		temp->selectedIndices.insert(i);
	return temp;
}

shared_ptr<Selection> Selection::getSubtraction(shared_ptr<Selection> first, shared_ptr<Selection> second)
{
	shared_ptr<Selection> temp = make_shared<Selection>();
	// Get all selections for first sets
	temp->selectedIndices = first->selectedIndices;
	// Remove any that are also in the second sets
	for each(int i in second->selectedIndices)
		temp->selectedIndices.erase(i);
	return temp;
}

shared_ptr<Selection> Selection::getIntersection(shared_ptr<Selection> a, shared_ptr<Selection> b)
{
	shared_ptr<Selection> temp = make_shared<Selection>();
	// Add sprite and nt indices if both selections have them
	for each(int i in a->selectedIndices)
	{
		if(b->selectedIndices.count(i) > 0)
			temp->selectedIndices.insert(i);
	}
	return temp;
}

void Selection::render(vector<SceneSprite> * sprites, int moveX, int moveY)
{
	// Render all OAM highlights
	Overlay::setColor(0.0f, 1.0f, 0.0f, 1.0f);
	for each(int i in selectedIndices)
	{
		SceneSprite s = sprites->at(i);
		Overlay::drawSpriteSquare(s.x + moveX, s.y + moveY);
	}
}

Vector2D Scene::getTopLeftSpriteInSelection(shared_ptr<Selection> s)
{
	int x = 100000;
	int y = 100000;
	for each(int i in s->selectedIndices)
	{
		if (sprites[i].x < x)
			x = sprites[i].x;
		if (sprites[i].y < y)
			y = sprites[i].y;
	}
	return { x, y };
}