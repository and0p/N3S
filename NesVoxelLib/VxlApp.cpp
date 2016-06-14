#include "stdafx.h"
#include "VxlApp.h"

VxlApp::VxlApp(VxlD3DContext context)
{
	VxlUtil::initPipeline(context);
	gameData = std::shared_ptr<VoxelGameData>(new VoxelGameData(512, 1));
	camera = VxlCamera();
}

void VxlApp::load()
{
	NesEmulator::Initialize();
	info = NesEmulator::getGameInfo();
	gameData->grabBitmapSprites(info->data, 32784);
	gameData->createSpritesFromBitmaps();
	gameData->buildAllMeshes();
}

void VxlApp::update()
{
	NesEmulator::ExecuteFrame();
	snapshot.reset(new VxlPPUSnapshot((VxlRawPPU*)NesEmulator::getVRam()));
}

void VxlApp::render()
{
	VxlUtil::updateGameTexture(NesEmulator::getPixelData());
	camera.SetPosition(2.0, 0.0f, -1.0f);
	camera.SetRotation(-65.0f, 0.0f, 0);
	camera.Render();
	VxlUtil::updateMatricesWithCamera(&camera);
	VxlUtil::updateWorldMatrix(0.0f, 0.0f, 0.0f);
	//VxlUtil::renderMesh(testMesh);
	for (int i = 0; i < 64; i++) {
		int x = snapshot->sprites[i].x;
		int y = snapshot->sprites[i].y;
		int tile = snapshot->sprites[i].tile;
		float posX, posY;
		if (y > 0 && y < 240) {
			posX = -1.0f + (pixelSizeW * x);
			posY = 1.0f - (pixelSizeH * y);
			VxlUtil::updateWorldMatrix(posX, posY, -0.2f);
			if (gameData->sprites[tile].meshExists)
			{
				VxlUtil::renderMesh(gameData->sprites[tile].mesh);
			}
		}
	}
	for (int i = 0; i < 960; i++) {
		int x = i % 32;
		int y = floor(i / 32);
		float posX, posY;
		posX = -1.0f + (pixelSizeW * x * 8);
		posX -= (pixelSizeW * snapshot->ppuScroll);
		posY = 1.0f - (pixelSizeH * y * 8);
		VxlUtil::updateWorldMatrix(posX, posY, -0.2f);
		int tile = snapshot->nameTables->operator[](0).tiles[i].tile + 256;
		if (gameData->sprites[tile].meshExists)
		{
			VxlUtil::renderMesh(gameData->sprites[tile].mesh);
		}
	}
}