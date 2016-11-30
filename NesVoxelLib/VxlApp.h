#pragma once

#include "VxlUtil.h"
#include "N3SGameData.hpp"
#include "VxlPPUSnapshot.h"
#include "NesEmulator.h"
#include "VxlD3DContext.h"
#include "VxlCamera.h"
#include "libretro.h"
#include "VxlInput.h"
#include "N3SPatternTable.hpp"
#include "VxlAudio.h"
#include <memory>
#include <iostream>
#include <fstream>

class VxlApp {
public:
	VxlApp();
	void assignD3DContext(VxlD3DContext);
	void initDirectAudio(HWND hwnd);
	void load(string path);
	void loadGameData(string path);
	void unload();
	void reset();
	void update(bool runThisFrame);
	void render();
	void pause();
	void unpause();
	void setMute(bool mute);
	void updateCameraViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection);
	void updateGameOriginPosition(float x, float y, float z);
	void recieveKeyInput(int key, bool down);
	XMVECTORF32 getBackgroundColor();
	retro_game_info *info;
	VxlCamera camera;
	std::shared_ptr<GameData> gameData;
	bool loaded;
	bool save();
	bool saveAs(string path);
private:
	string romPath;
	string n3sPath;
	SoundDriver *audioEngine;
	HWND hwnd;
	bool emulationPaused;
	bool pausedThisPress;
	bool frameAdvanced;
	bool muted;
	std::shared_ptr<VxlPPUSnapshot> snapshot;
	InputState inputState;
	shared_ptr<VirtualPatternTable> virtualPatternTable;
	void updatePalette();
	void renderSprites();
	void renderSprite(shared_ptr<VirtualSprite> vSprite, int x, int y, int palette, bool flipX, bool flipY);
	void renderNameTables();
	void renderScrollSection(ScrollSection section);
	void renderRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect);
};