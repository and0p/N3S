#pragma once
#include "VxlPPUSnapshot.h"
#include "VxlAudio.h"

class NesEmulator {
public:
	static int16_t inputs[2][8];
	static void Initialize(const char * romPath);
	static void ExecuteFrame();
	static void unload();
	static void reset();
	static const void* getPixelData();
	static const void* getVRam();
	static struct retro_game_info* getGameInfo();
	static SoundDriver* audioEngine;
};