#pragma once

#include "VxlUtil.h"
#include "VxlGameData.h"
#include "VxlPPUSnapshot.h"
#include "NesEmulator.h"
#include "VxlD3DContext.h"
#include "VxlCamera.h"
#include "libretro.h"
#include <memory>

class VxlApp {
public:
	VxlApp();
	void assignD3DContext(VxlD3DContext);
	void load();
	void update();
	void render();
	retro_game_info *info;
	VxlCamera camera;
	std::shared_ptr<VoxelGameData> gameData;
	std::shared_ptr<VxlPPUSnapshot> snapshot;
};