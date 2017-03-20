#pragma once

#include "N3sApp.hpp"
#include "PpuSnapshot.hpp"

class GameView {
public:
	static void update();
	static void render();
	static FreeCamera * getCamera();
private:
	static void parseInput();
};

void updatePalette();
void renderSprites();
void renderSprite(shared_ptr<VirtualSprite> vSprite, int x, int y, int palette, bool flipX, bool flipY);
void renderNameTables();
void renderScrollSection(ScrollSection section);
void renderRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect);