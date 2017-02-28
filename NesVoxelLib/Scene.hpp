#pragma once
#include "stdafx.h"
#include <vector>
#include "N3sApp.hpp"
#include "N3sPalette.hpp"

using namespace std;

const int sceneWidth = 64;
const int sceneHeight = 60;

const int scenePixelWidth = 512;
const int scenePixelHeight = 480;

const int sceneDXWidth = pixelSizeW * 8 * sceneWidth;
const int sceneDXHeight = pixelSizeH * 8 * sceneHeight;

enum MouseModifier { no_mod, shift_mod, alt_mod, ctrl_mod };
enum MouseFunction { no_func, move_func, select_new, select_add, select_sub, select_intersect };

struct SceneSprite {
	int meshNum;
	int palette;
	int x;
	int y;
	bool mirrorH;
	bool mirrorV;
};

class SceneBackground {
public:
	SceneSprite sprites[sceneWidth * sceneHeight];
};

class Highlight {
public:
	int selectedIndex = -1;
	vector<int> highlightedSpriteIndices;
	int highlightedBackgroundIndex = -1;
	void clear();
	int getHighlightedOAM();
	int getHighlightedNT();
};

class Selection {
public:
	unordered_set<int> selectedSpriteIndices;
	unordered_set<int> selectedBackgroundIndices;
	void clear();
};

class Scene {
public:
	Scene();
	bool update(bool mouseAvailable);
	void render(bool renderBackground, bool renderOAM);
	void renderOverlays(bool drawBackgroundGrid, bool drawOamHighlights);
	vector<SceneSprite> sprites;
	void setBackgroundSprite(int x, int y, SceneSprite sprite);
	void addOAMSprite(SceneSprite s);
	static void createSceneFromCurrentSnapshot();
	N3sPalette palettes[8];
	int selectedPalette = 0;
	N3sPalette * getSelectedPalette();
	void selectNextPalette();
	void selectPreviousPalette();
	Highlight highlight;
	Selection selection;
	void updateHighlight2d(int x, int y, bool highlightOAM, bool highlightNametable);
	bool updateMouseActions(bool mouseAvailable);
private:
	SceneSprite bg[sceneWidth * sceneHeight];
	static void getCoordinatesFromZIntersection();
};