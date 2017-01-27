#pragma once
#include <vector>
#include "N3sApp.hpp"
#include "N3sPalette.hpp"

using namespace std;

const int sceneWidth = 64;
const int sceneHeight = 60;

struct SceneSprite {
	int meshNum;
	int palette;
	bool mirrorH;
	bool mirrorV;
};

class SceneBackground {
public:
	SceneSprite sprites[sceneWidth * sceneHeight];
};

class Scene {
public:
	Scene();
	void render();
	vector<SceneSprite> sprites;
	void setBackgroundSprite(int x, int y, SceneSprite sprite);
	static void createSceneFromCurrentSnapshot();
	N3sPalette palettes[8];
	int selectedPalette = 0;
private:
	SceneSprite bg[sceneWidth * sceneHeight];
};