#pragma once
#include "Camera.hpp"
#include "Scene.hpp"

using namespace std;
const int sceneCount = 16;

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
	static void parseInputForScene(bool mouseAvailable);
	static void render();
	static void setScene(int s);
	static Hue getBackgroundColor();
	static interactionOptions oamOptions;
	static interactionOptions nametableOptions;
	static void updateTempScene(shared_ptr<PpuSnapshot> snapshot);
private:
	static shared_ptr<vector<SceneSprite>> copiedSprites;
	static bool mouseAvailable;
};