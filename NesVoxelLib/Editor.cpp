#include "stdafx.h"
#include "Editor.hpp"

shared_ptr<Scene> scenes[8];

int selectedScene = 0;
int selectedSprite;
bool editing;
Camera mainCamera;

void Editor::init()
{
	for (int i = 0; i < 8; i++)
	{
		scenes[i] = make_shared<Scene>();
	}
	scenes[0]->setBackgroundSprite(0, 0, { 1,0,false,false });
}

void Editor::update()
{
	
}

void Editor::render()
{
	// Update view with whatever camera
	scenes[selectedScene]->render();
	// Optional: render editor stuff if things are selected / highlighted
	// Render UI
}
