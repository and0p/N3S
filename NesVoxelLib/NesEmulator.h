#pragma once
#include "VxlPPUSnapshot.h"

class NesEmulator {
public:
	static void Initialize(void);
	static void ExecuteFrame();
	static const void* getPixelData();
	static const void* getVRam();
	static void takeSnapshot();
	static VxlPPUSnapshot snapshot;
	static struct retro_game_info* getGameInfo();
};