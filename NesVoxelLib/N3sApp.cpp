#include "stdafx.h"
#include "N3sApp.hpp"
#include <time.h>
#include "N3sConsole.hpp"
#include "Overlay.hpp"
#include "Editor.hpp"
#include "N3sPalette.hpp"
#include "N3sConfig.hpp"
#include "FileUtil.hpp"
#include "SaveState.hpp"
#include <iostream>
#include <fstream>

extern SoundDriver *newDirectSound();

shared_ptr<GameData> N3sApp::gameData;
shared_ptr<PpuSnapshot> N3sApp::snapshot;
shared_ptr<InputState> N3sApp::inputState;
shared_ptr<VirtualPatternTable> N3sApp::virtualPatternTable;
string N3sApp::applicationDirectory;
N3sRomConnection conn;

void (*throwAlert)(string message, string caption) = NULL;

N3sApp::N3sApp()
{
	this->hwnd = hwnd;
	gameData = {};
	emulationPaused = false;
	loaded = false;
	muted = false;
	inputState = make_shared<InputState>();
	SoundDriver * drv = 0;
	N3sPalette::init();
	N3sConsole::init();
	// Get the application directory
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	wstring ws(buffer);
	applicationDirectory = string(ws.begin(), ws.end());
	GameData::initMaps();
	// Apply configs from file, if it exists
	N3sConfig::init();
	N3sConfig::load();
	State::clear();
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
	conn = N3sRomConnection(path);
	if (conn.isValid())
	{
		try
		{
			NesEmulator::Initialize(conn.getRomCStr());
			info = NesEmulator::getGameInfo();
		}
		catch (...)
		{
			showAlert("Error loading NES rom.", "Error");
			conn.n3sFileExists = false;
			return;
		}
		if (conn.n3sFileExists)
		{
			loadGameData(conn.getN3sString(), true);
		}
		else
		{
			// TODO: Ask if user wants to find N3S file, generate data, or cancel
			gameData = make_shared<GameData>((char*)info->data);
			n3sPath = replaceExt(romPath, "n3s");
			Editor::init();
		}
		virtualPatternTable = shared_ptr<VirtualPatternTable>(new VirtualPatternTable(gameData));
		// Get the size of the save state buffer
		State::init(conn.getRomN3sPath(), NesEmulator::getStateBufferSize());
		// gameData->grabBitmapSprites(info->data);
		// gameData->createSpritesFromBitmaps();
		loaded = true;
		unpause();
	}
	else
	{
		// TODO error if file is not good
	}
}

void N3sApp::loadGameData(string path, bool init)
{
	// If not initializing rom, only go forward if game is loaded
	if ((!init && loaded) || init)
	{
		ifstream n3sFile(path.c_str());
		// Make sure file exists
		if (n3sFile.good())
		{
			// Unload all old assets
			if(loaded)
				gameData->unload();
			// Load the file into json
			json input = json::parse(n3sFile);
			// Close file
			n3sFile.close();
			// Make GameData from json
			gameData.reset(new GameData((char*)info->data, input["gamedata"]));
			virtualPatternTable.reset(new VirtualPatternTable(gameData));	// Reset to reference new game data
			// Load Editor data from json
			Editor::loadJSON(input["editor"]);
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
	inputState->refreshInput();
	// Update console
	N3sConsole::update();
	// See if we're switching modes due to user input
	if (N3sApp::inputState->functions[tog_game]->activatedThisFrame)
	{
		mode = gameMode;
		audioEngine->resume();
		N3sConsole::writeLine("SWITCHED TO GAME MODE");
	}
	else if (N3sApp::inputState->functions[tog_editor]->activatedThisFrame)
	{
		if(mode != editorMode)
			Editor::updateTempScene(snapshot);
		mode = editorMode;
		audioEngine->pause();
		N3sConsole::writeLine("SWITCHED TO EDITOR MODE");
	}
	// See if a game is loaded
	if (loaded)
	{
		bool advancingFrameManually = inputState->functions[emu_advance_frame]->activatedThisFrame;
		// See if we're in gameplay or editor mode and update respective view
		switch (mode)
		{
		case (gameMode):
			// If we are loaded and emulating, run the game and update PPU data
			if (inputState->functions[emu_nextstate]->activatedThisFrame)
				State::incrementSlot();
			if (inputState->functions[emu_prevstate]->activatedThisFrame)
				State::decrementSlot();
			if (inputState->functions[emu_savestate]->activatedThisFrame)
				State::save();
			if (inputState->functions[emu_loadstate]->activatedThisFrame)
			{
				if (State::load())
					loadedStateThisFrame = true;
			}
			if ((!runThisNesFrame && (!emulationPaused || (emulationPaused && advancingFrameManually))) || loadedStateThisFrame)
			{
				if(!loadedStateThisFrame)
					NesEmulator::ExecuteFrame();
				snapshot.reset(new PpuSnapshot((N3sRawPpu*)NesEmulator::getVRam()));
				virtualPatternTable->update(snapshot->patternTable);
				renderBatch.reset(new RenderBatch(gameData, snapshot, virtualPatternTable));
				loadedStateThisFrame = false;
			}
			GameView::update();
			break;
		case (editorMode):
			Editor::update();
			renderBatch.reset(new RenderBatch(gameData, Editor::getSelectedScene()));	// TODO make this partial update when possible
			break;
		}
		// Update the current configuration
		N3sConfig::update(snapshot);
	}
	else
	{
		N3sConfig::update(nullptr);
	}

}

void N3sApp::render()
{
	if (loaded)
	{
		N3s3d::clear(); // Reset anything that needs to be reset in D3D context
		// See if we're in gameplay or editor mode and render respective view
		switch (mode)
		{
		case (gameMode):
			renderBatch->render(GameView::getCamera());
			break;
		case (editorMode):
			renderBatch->render(Editor::getCamera());
			Editor::render();
			break;
		}
	}
	// Render the console
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

void N3sApp::togglePause()
{
	if (emulationPaused)
		unpause();
	else
		pause();
}

void N3sApp::setMute(bool mute)
{
	muted = mute;
	if (muted)
		audioEngine->pause();
	else
		audioEngine->resume();
	N3sConfig::setOption(mute_audio, muted);
}

void N3sApp::toggleMute()
{
	muted ? setMute(false) : setMute(true);
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
		inputState->keyboardMouse->setDown(key);
	else
		inputState->keyboardMouse->setUp(key);
}

void N3sApp::recieveMouseInput(MouseButtons button, bool down)
{
	InputState::keyboardMouse->mouseButtons[button].down = down;
}

void N3sApp::recieveMouseMovement(int x, int y)
{
	InputState::keyboardMouse->mouseX = x;
	InputState::keyboardMouse->mouseY = y;
}

void N3sApp::recieveMouseScroll(int delta)
{
	InputState::keyboardMouse->wheelDelta = delta;
}

void N3sApp::setAlertFunction(void (*f)(string message, string caption))
{
	throwAlert = f;
}

void N3sApp::showAlert(string message, string caption)
{
	throwAlert(message, caption);
}

InputConfig N3sApp::getInputConfig()
{
	return InputState::getInputConfig();
}

bool N3sApp::applyInputConfig()
{
	return false;
}

XMVECTORF32 N3sApp::getBackgroundColor()
{
	Hue hue;
	if (loaded)
	{
		switch (mode)
		{
		case (gameMode):
			hue = snapshot->palette.getBackgroundColor();
			break;
		case (editorMode):
			hue = Editor::getBackgroundColor();
			break;
		}
	}
	else
		hue = { 0.0f, 0.0f, 0.0f };
	return{ hue.red, hue.green, hue.blue, 1.0f };
}

bool N3sApp::save()
{
	return saveAs(n3sPath);
}

bool N3sApp::saveAs(string path)
{
	if (loaded)
	{
		// Get output JSON
		json j;
		j["gamedata"] = gameData->getJSON();
		j["editor"] = Editor::getJSON();
		string output = j.dump(4);
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