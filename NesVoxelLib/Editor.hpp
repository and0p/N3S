#pragma once
#include "Camera.hpp"
#include "Scene.hpp"

using namespace std;

struct interactionOptions
{
	bool display = true;
	bool highlightAll = false;
	enum selectionType
	{
		select_none, select_2d, select3d
	};
};

class Editor {
public:
	static void init();
	static void update();
	static void render();
	static void setScene(int s);
	static Hue getBackgroundColor();
private:
	static interactionOptions oamOptions;
	static interactionOptions namtetableOptions;
	static bool mouseAvailable;
	static XMVECTOR mouseVector;
};