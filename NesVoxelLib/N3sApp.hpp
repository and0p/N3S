#pragma once

#include "N3s3d.hpp"
#include "GameData.hpp"
#include "PpuSnapshot.hpp"
#include "NesEmulator.hpp"
#include "libretro.h"
#include "Input.hpp"
#include "N3sPatternTable.hpp"
#include "Audio.hpp"
#include "Overlay.hpp"
#include "GameView.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include "Windows.h"
#include "RenderBatch.hpp"

enum n3sMode { gameMode, editorMode };

class N3sApp {
public:
	N3sApp();
	void assignD3DContext(N3sD3dContext);
	void initDirectAudio(HWND hwnd);
	void load(string path);
	void loadGameData(string path, bool init);
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
	void recieveMouseInput(MouseButtons button, bool down);
	void recieveMouseMovement(int x, int y);
	void recieveMouseScroll(int delta);
	InputConfig getInputConfig();
	bool applyInputConfig();
	XMVECTORF32 getBackgroundColor();
	retro_game_info *info;
	static shared_ptr<GameData> gameData;
	static shared_ptr<PpuSnapshot> snapshot;
	static shared_ptr<InputState> inputState;
	static shared_ptr<VirtualPatternTable> virtualPatternTable;
	bool loaded;
	bool save();
	bool saveAs(string path);
	n3sMode mode = gameMode;
	static string applicationDirectory;
private:
	string romPath;
	string n3sPath;
	SoundDriver *audioEngine;
	shared_ptr<RenderBatch> renderBatch;
	HWND hwnd;
	bool emulationPaused;
	bool muted;

};