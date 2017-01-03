#pragma once
#include <vector>
#include "N3sApp.hpp"

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
private:
	SceneSprite bg[sceneWidth * sceneHeight];
};