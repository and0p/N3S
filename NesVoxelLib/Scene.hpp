#pragma once

#include <vector>
#include "N3sPalette.hpp"
#include "VoxelEditor.hpp"
#include "N3sMath.hpp"

using namespace std;
using json = nlohmann::json;

const int sceneWidth = 64;
const int sceneHeight = 60;

const int scenePixelWidth = 512;
const int scenePixelHeight = 480;

const int sceneDXWidth = pixelSizeW * 8 * sceneWidth;
const int sceneDXHeight = pixelSizeW * 8 * sceneHeight;

const int bgSize = sceneWidth * sceneHeight;

enum MouseModifier { no_mod, mod_add, mod_remove, mod_intersect, mod_copy };
enum MouseFunction { no_func, move_func, select_new, select_add, select_sub, select_intersect };

enum SelectionDirection { select_up, select_down, select_left, select_right };

class SceneBackground {
public:
	SceneSprite sprites[sceneWidth * sceneHeight];
};

class Highlight {
public:
	unordered_set<int> highlightedIndices;
	int getHighlight();
	void clear();
	bool anythingHighlighted;
};

class Selection {
public:
	unordered_set<int> selectedIndices;
	void clear();
	//bool anythingSelected;
	static shared_ptr<Selection> getUnion(shared_ptr<Selection> a, shared_ptr<Selection> b);
	static shared_ptr<Selection> getSubtraction(shared_ptr<Selection> first, shared_ptr<Selection> second);
	static shared_ptr<Selection> getIntersection(shared_ptr<Selection> a, shared_ptr<Selection> b);
	void render(vector<SceneSprite> * sprites, int moveX, int moveY);
};

class Scene {
public:
	Scene();
	Scene(shared_ptr<PpuSnapshot> snapshot);
	Scene(json j);
	json getJSON();
	bool update(bool mouseAvailable, shared_ptr<OrbitCamera> cam);
	void render(bool renderBackground, bool renderOAM);
	void renderOverlays(bool drawBackgroundGrid, bool drawOamHighlights);
	void changeSelectionPalette(int p);
	vector<SceneSprite> sprites;
	void addSprite(SceneSprite s);
	N3sPalette palettes[8];
	int selectedPalette = 0;
	N3sPalette * getSelectedPalette();
	void selectNextPalette();
	void selectPreviousPalette();
	Highlight highlight;
	shared_ptr<Selection> selection;
	shared_ptr<Selection> displaySelection;
	void updateHighlight2d(Vector2D mouse, bool highlightOAM, bool highlightNametable);
	bool updateMouseActions(bool mouseAvailable);
	void moveSelection(bool copy);
	shared_ptr<VoxelEditor> voxelEditor;
	shared_ptr<SpriteMesh> getSelectedMesh();
	Vector2D dragStart;
	Vector2D dragDestination;
	shared_ptr<vector<SceneSprite>> copySelection();
	void pasteSelection(shared_ptr<vector<SceneSprite>> copiedSprites);
	int findNearestSprite(int selected, SelectionDirection direction);
	int spriteBeingEdited = -1;
private:
	void checkKeyInput();
	void moveSelection(int x, int y);
	void deleteSelection();
	bool showGuides = false;
	static Vector3F Scene::getCoordinatesFromZIntersection(XMFLOAT3 zIntersect);
	Vector2D getTopLeftSpriteInSelection(shared_ptr<Selection> s);
};