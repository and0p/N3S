#pragma once
#include "PpuSnapshot.hpp"
#include "Audio.hpp"

class NesEmulator {
public:
	static int16_t inputs[2][16];
	static void Initialize(const char * romPath);
	static void ExecuteFrame();
	static void unload();
	static void reset();
	static const void* getPixelData();
	static const void* getVRam();
	static struct retro_game_info* getGameInfo();
	static SoundDriver* audioEngine;
	static size_t getStateBufferSize();
	static bool saveState(void *output, size_t size);
	static bool loadState(const void *input, size_t size);
};