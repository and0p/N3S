#pragma once
#include "Camera.hpp"
#include "Scene.hpp"

class Editor {
public:
	static void init();
	static void update();
	static void render();
	static void setScene(int s);
private:
	static bool mouseAvailable;
};