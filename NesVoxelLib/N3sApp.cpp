#include "stdafx.h"
#include "N3sApp.hpp"
#include <time.h>
#include "N3sConsole.hpp"
#include "Overlay.hpp"
#include "Editor.hpp"

extern SoundDriver *newDirectSound();

shared_ptr<GameData> N3sApp::gameData;
shared_ptr<PpuSnapshot> N3sApp::snapshot;
shared_ptr<InputState> N3sApp::inputState;
shared_ptr<VirtualPatternTable> N3sApp::virtualPatternTable;

N3sApp::N3sApp()
{
	this->hwnd = hwnd;
	gameData = {};
	emulationPaused = false;
	loaded = false;
	muted = false;
	camera.SetPosition(0, 0, -2);
	camera.SetRotation(0, 0, 0);
	inputState = make_shared<InputState>();
	SoundDriver * drv = 0;
	N3sConsole::init();
}

void N3sApp::assignD3DContext(N3sD3dContext context)
{
	N3s3d::initPipeline(context);
	Overlay::init();
}

void N3sApp::initDirectAudio(HWND hwnd)
{
	audioEngine = newDirectSound();
	audioEngine->init(hwnd, 44100);
	NesEmulator::audioEngine = audioEngine;
	audioEngine->resume();
}

void N3sApp::load(string path)
{
	if (loaded)
		unload();
	// Convert string to char * for libretro loading function
	const char* cPath = path.c_str(); // TODO leak?
	NesEmulator::Initialize(cPath);
	romPath = path;
	info = NesEmulator::getGameInfo();
	// See if a matching N3S file exists with same name as ROM
	// TODO: Doing it in a really dumb way at the moment, stripping nes and replacing with n3s
	string s = path;
	int length = s.length();
	s[length - 2] = '3';
	ifstream n3sFile(s.c_str());
	if (n3sFile.good())
	{
		n3sPath = s;
		json input(n3sFile);
		// s = input.dump(4);
		n3sFile.close();
		gameData = make_shared<GameData>((char*)info->data, input);
	}
	else
	{
		// TODO: Ask if user wants to find N3S file, generate data, or cancel
		gameData = make_shared<GameData>((char*)info->data);
		n3sPath = replaceExt(romPath, "n3s");
	}
	virtualPatternTable = shared_ptr<VirtualPatternTable>(new VirtualPatternTable(gameData));
	// gameData->grabBitmapSprites(info->data);
	// gameData->createSpritesFromBitmaps();
	loaded = true;
	Editor::init();
	unpause();
}

void N3sApp::loadGameData(string path)
{
	// Make sure game is loaded first
	if (loaded)
	{
		ifstream n3sFile(path.c_str());
		// Make sure file exists
		if (n3sFile.good())
		{
			// Unload all old assets
			gameData->unload();
			// Load the file into json
			json input(n3sFile);
			// Close file
			n3sFile.close();
			// Make GameData from json
			gameData.reset(new GameData((char*)info->data, input));
			virtualPatternTable.reset(new VirtualPatternTable(gameData));	// Reset to reference new game data
		}
	}
}

void N3sApp::unload()
{
	if (loaded)
	{
		pause();
		gameData->unload();
		gameData.reset();
		virtualPatternTable.reset();
		camera.SetPosition(0, 0, -2);
		camera.SetRotation(0, 0, 0);
		loaded = false;
	}
}

void N3sApp::reset()
{
	NesEmulator::reset();
}

void N3sApp::update(bool runThisNesFrame)
{
	// Update input
	inputState->checkGamePads();
	inputState->refreshInput();
	// Update console
	N3sConsole::update();
	// See if we're switching modes due to user input
	if (N3sApp::inputState->keyboardState.keyStates[VK_OEM_COMMA])
		mode = gameMode;
	if (N3sApp::inputState->keyboardState.keyStates[VK_OEM_PERIOD])
		mode = editorMode;
	// See if a game is loaded
	if (loaded)
	{
		// See if we're in gameplay or editor mode and update respective view
		switch (mode)
		{
		case (gameMode):
			// If we are loaded and emulating, run the game and update PPU data
			if (!runThisNesFrame && !emulationPaused)
			{
				NesEmulator::ExecuteFrame();
				snapshot.reset(new PpuSnapshot((N3sRawPpu*)NesEmulator::getVRam()));
				virtualPatternTable->update(snapshot->patternTable);
			}
			GameView::update();
			break;
		case (editorMode):
			Editor::update();
			break;
		}
	}	
}

void N3sApp::render()
{
	if (loaded)
	{
		// See if we're in gameplay or editor mode and render respective view
		switch (mode)
		{
		case (gameMode):
			GameView::render();
			break;
		case (editorMode):
			Editor::render();
			break;
		}
	}
	/*
	// Overlay shader testing
	N3s3d::setDepthBufferState(false);
	N3s3d::setShader(overlay);
	N3s3d::updateMatricesWithCamera(&camera);
	// Overlay::drawVoxelPreview(0, 0, 0);
	// Overlay::drawVoxelPreview(5, 5, 0);
	// Overlay::drawVoxelPreview(-128, -120, 0);
	Overlay::drawVoxelGrid(0, 0, 0, xAxis);
	Overlay::drawVoxelGrid(0, 0, 4, yAxis);
	Overlay::drawVoxelGrid(0, 0, 4, zAxis);
	Overlay::drawVoxelGrid(16, 15, 0, xAxis);
	Overlay::drawVoxelGrid(16, 15, 4, yAxis);
	Overlay::drawVoxelGrid(16, 15, 4, zAxis);
	Overlay::drawNametableGrid();
	N3s3d::setGuiProjection();
	*/
	N3sConsole::render();
}

void N3sApp::pause()
{
	emulationPaused = true;
	audioEngine->pause();
}

void N3sApp::unpause()
{
	emulationPaused = false;
	audioEngine->resume();
}

void N3sApp::setMute(bool mute)
{
	muted = mute;
	if (muted)
		audioEngine->pause();
	else
		audioEngine->resume();
}

void N3sApp::updateCameraViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	N3s3d::updateViewMatrices(view, projection);
}

void N3sApp::updateGameOriginPosition(float x, float y, float z)
{

}

void N3sApp::recieveKeyInput(int key, bool down)
{
	if (down)
		inputState->keyboardState.setDown(key);
	else
		inputState->keyboardState.setUp(key);
}

XMVECTORF32 N3sApp::getBackgroundColor()
{
	Hue hue;
	if (loaded)
		hue = N3s3d::ppuHueStandardCollection.getHue(v2C02, 0, snapshot->backgroundColor);
	else
		hue = { 0.0f, 0.0f, 0.0f };
	return{ hue.red, hue.green, hue.blue, 1.0f };
}

bool N3sApp::save()
{
	if (loaded)
	{
		// Get output JSON
		string output = gameData->getJSON();
		// Save to file with path specified
		ofstream myfile;
		myfile.open(n3sPath);
		myfile << output;
		myfile.close();
		return true;
	}
	return false;
}

bool N3sApp::saveAs(string path)
{
	if (loaded)
	{
		// Get output JSON
		string output = gameData->getJSON();
		// Save to file with path specified
		ofstream myfile;
		myfile.open(path);
		myfile << output;
		myfile.close();
		// Replace path to "loaded" N3S file
		n3sPath = path;
		return true;
	}
	return false;
}

// thx https://www.safaribooksonline.com/library/view/c-cookbook/0596007612/ch10s17.html
string replaceExt(string input, string newExt) {
	string::size_type i = input.rfind('.', input.length());

	if (i != string::npos) {
		input.replace(i + 1, newExt.length(), newExt);
	}
	return input;
}