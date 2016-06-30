#include "stdafx.h"
#include "VxlApp.h"

VxlApp::VxlApp()
{
	
	gameData = std::shared_ptr<VoxelGameData>(new VoxelGameData(512, 1));
	camera = VxlCamera();
}

void VxlApp::assignD3DContext(VxlD3DContext context)
{
	VxlUtil::initPipeline(context);
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
	camera.SetPosition(0.5f, 0.0f, -2.0f);
	camera.SetRotation(-15.0f, 0.0f, 0);
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
	for (int y = 0; y < 30; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			float posX, posY;
			posX = -1.0f + (pixelSizeW * x * 8);
			posX -= (pixelSizeW * snapshot->ppuScroll);
			posY = 1.0f - (pixelSizeH * y * 8);
			VxlUtil::updateWorldMatrix(posX, posY, -0.2f);
			int tile = snapshot->background.getTile(x, y, 0).tile;
			if (tile < 0)
				tile = 512 + tile;
			else
				tile += 256;
			if (gameData->sprites[tile].meshExists)
			{
				VxlUtil::renderMesh(gameData->sprites[tile].mesh);
			}
		}
	}
}

void renderNameTables()
{

}

void renderScrollSection(VxlPPUSnapshot snapshot, int scrollSectionNumber)
{
	ScrollSection section = snapshot.scrollSections[scrollSectionNumber];
	// Get offset of top-left pixel within top-left sprite referenced
	int xOffset = section.x % 8;
	int yOffset = section.y % 8;
	// Count height of section
	int sectionHeight = section.bottom - section.top;
	// Figure out how much to clip the sprites on the edges, if at all
	Sides offset = { 0, 0, 0, 0 };
	if (yOffset > 0)
	{
		offset.top = 8 - yOffset;
		offset.bottom = (sectionHeight - yOffset) % 8;
	}
	if (xOffset > 0)
	{
		offset.left = 8 - xOffset;
		offset.right = (256 - xOffset) % 8;
	}
	
}