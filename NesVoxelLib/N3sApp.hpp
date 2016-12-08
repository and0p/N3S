#pragma once

#include "N3s3d.hpp"
#include "GameData.hpp"
#include "PpuSnapshot.hpp"
#include "NesEmulator.hpp"
#include "Camera.hpp"
#include "libretro.h"
#include "Input.hpp"
#include "N3sPatternTable.hpp"
#include "Audio.hpp"
#include "Overlay.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include "N3sConsole.hpp"

class N3sApp {
public:
	N3sApp();
	void assignD3DContext(N3sD3dContext);
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
	Camera camera;
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
	std::shared_ptr<PpuSnapshot> snapshot;
	InputState inputState;
	shared_ptr<VirtualPatternTable> virtualPatternTable;
	void updatePalette();
	void renderSprites();
	void renderSprite(shared_ptr<VirtualSprite> vSprite, int x, int y, int palette, bool flipX, bool flipY);
	void renderNameTables();
	void renderScrollSection(ScrollSection section);
	void renderRow(int y, int height, int xOffset, int yOffset, int nametableX, int nametableY, int nameTable, bool patternSelect);
};