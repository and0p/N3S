#pragma once

#include "N3sPalette.hpp"
#include "Scene.hpp"

using namespace std;

enum PaletteButtons {
	palette_start = 0,
	palette_end = 23,
	bg_swatch = 24,
	colors_start = 25,
	colors_end = 89,
	colors_close = 90,
	next_palettte = 91,
	previous_palette = 92,
	options_open = 93,
	options_close = 94
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
	static const int buttonHeight = 20;
	static const int buttonWidth = 20;
	static const int buttonGap = 10;
};

class PaletteSelector
{
public:
	PaletteSelector() {}
	bool update(bool mouseAvailable) { return false; }
	bool update(bool mouseAvailable, shared_ptr<Scene> scene );
	void render(shared_ptr<Scene> scene);
private:
	bool mouseCaptured = false;
	bool open = false;
	int highlightedIndex = -1;
	int selectedIndex = -1;
	static const int leftMargin = 40;
	static const int boxSize = 30;
	static const int borderSize = 2;
	static const int swatchSize = boxSize - (borderSize * 2);
};

inline bool mouseInRectangle(int mouseX, int mouseY, int rectX, int rectY, int rectWidth, int rectHeight) {
	if (mouseX >= rectX && mouseX < rectX + rectWidth && mouseY >= rectY && mouseY < rectY + rectHeight)
		return true;
	else
		return false;
}