#pragma once

#include "N3sPalette.hpp"
#include "Scene.hpp"
#include "VoxelEditor.hpp"

using namespace std;

enum SceneButtons {
	temp_scene = 17
};

enum PaletteButtons {
	palette_start = 0,
	palette_end = 23,
	bg_swatch = 24,
	colors_start = 25,
	colors_end = 89,
	colors_close = 90,
	next_palettte = 91,
	previous_palette = 92,
	options_toggle = 93,
	eraser = 94
};

class GuiElement
{
public:
	virtual bool update(bool mouseAvailable) = 0;
	virtual void render() = 0;
};

class SceneSelector
{
public:
	bool update(bool mouseAvailable);
	void render();
private:
	bool mouseCaptured = false;
	int highlightedTab = 0;
	int selectedSheet = 0;
	static const int buttonHeight = 20;
	static const int buttonWidth = 36;
	static const int buttonGap = 2;
};

class PaletteSelector
{
public:
	PaletteSelector() {}
	bool update(bool mouseAvailable) { return false; }
	bool update(bool mouseAvailable, shared_ptr<Scene> scene, shared_ptr<VoxelEditor> editor);
	void render(shared_ptr<Scene> scene, shared_ptr<VoxelEditor> editor);
private:
	bool mouseCaptured = false;
	bool open = false;
	bool optionsOpen = false;
	int highlightedIndex = -1;
	int selectedIndex = -1;
	int selectedPalette = -1;
	int selectionLevel = 0;
	static const int leftMargin = 40;
	static const int boxSize = 30;
	static const int borderSize = 2;
	static const int swatchSize = boxSize - (borderSize * 2);
	void selectColor(int color, shared_ptr<Scene> scene, shared_ptr<VoxelEditor> editor);
	int getRelativeColor(int i);
};

class MeshInfo
{
public:
	// MeshInfo() {}
	static bool update(bool mouseAvailable, shared_ptr<SpriteMesh> mesh);
	static void render();
	static void clear();
	static const int topMargin = (20 * 19);
private:
	static shared_ptr<SpriteMesh> m;

	static const int width = 8;
	static const int height = 3;
	static const int scale = 2;
};

class VoxelEditorInfo
{
public:
	static bool update(bool mouseAvailable, shared_ptr<VoxelEditor> editor);
	static void render();
	static void clear();
	static shared_ptr<VoxelEditor> e;
private:
	static const int topMargin = MeshInfo::topMargin + (17 * 4);
	static const int width = 8;
	static const int height = 5;
	static const int scale = 2;
};

inline bool mouseInRectangle(int mouseX, int mouseY, int rectX, int rectY, int rectWidth, int rectHeight)
{
	if (mouseX >= rectX && mouseX < rectX + rectWidth && mouseY >= rectY && mouseY < rectY + rectHeight)
		return true;
	else
		return false;
}