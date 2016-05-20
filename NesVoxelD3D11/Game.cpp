//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <iostream>

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game(): voxelGameData(VoxelGameData(512, 1))
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	VoxelUtil::initPipeline(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext(), m_deviceResources->GetD3DDevice1(), m_deviceResources->GetD3DDeviceContext1());
	camera = new VoxelCamera();
	testMesh = VoxelUtil::CreateRectangle(texture);
	marker = VoxelUtil::CreateSpriteMarker();

	NesEmulator::Initialize();
	retro_game_info *info = NesEmulator::getGameInfo();
	voxelGameData.grabBitmapSprites(info->data, 32784);
	voxelGameData.createSpritesFromBitmaps();
	voxelGameData.buildAllMeshes();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
	//delete snapshot;
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
	NesEmulator::ExecuteFrame();
	VoxelUtil::updateGameTexture(NesEmulator::getPixelData());
	const void* stuff = NesEmulator::getVRam();
	snapshot.reset(new VoxelPPUSnapshot(stuff));
    elapsedTime;
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

	OutputDebugStringA("Rendering!\n");

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
	camera->SetPosition(2.0, 0.0f, -1.0f);
	camera->SetRotation(-65.0f, 0.0f, 0);
	camera->Render();
	VoxelUtil::updateMatricesWithCamera(camera);
	VoxelUtil::updateWorldMatrix(0.0f, 0.0f, 0.0f);
	VoxelUtil::renderMesh(testMesh);
	for (int i = 0; i < 64; i++) {
		int x = snapshot->sprites[i].x;
		int y = snapshot->sprites[i].y;
		int tile = snapshot->sprites[i].tile;
		float posX, posY;
		if (y > 0 && y < 240) {
			posX = -1.0f + (pixelSizeW * x);
			posY = 1.0f - (pixelSizeH * y);
			VoxelUtil::updateWorldMatrix(posX, posY, -0.2f);
			if (voxelGameData.sprites[tile].meshExists)
			{
				VoxelUtil::renderMesh(voxelGameData.sprites[tile].mesh);
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
		VoxelUtil::updateWorldMatrix(posX, posY, -0.2f);
		int tile = snapshot->nameTables->operator[](0).tiles[i].tile + 256;
		if (voxelGameData.sprites[tile].meshExists)
		{
			VoxelUtil::renderMesh(voxelGameData.sprites[tile].mesh);
		}
	}
    context;

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // TODO: Initialize device dependent objects here (independent of window size).
    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
