//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <iostream>
#include "N3sD3DContext.h"
#include "InputWindow.h"
#include "Menu.h"

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game()
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

	N3sD3dContext c = { m_deviceResources->GetD3DDevice(), 
						m_deviceResources->GetD3DDevice1(), 
						m_deviceResources->GetD3DDeviceContext(),
						m_deviceResources->GetD3DDeviceContext1() };

    app.assignD3DContext(c);
	app.initDirectAudio(window);
	initMenu();
	//app.load(' ');

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	bool run = false;
    m_timer.Tick([&]()
    {
        Update(m_timer);
		app.update(run);
		run = true;
    });

	Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
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

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	// Let the app render
	app.render();

	//context;

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

	if(app.loaded)
		context->ClearRenderTargetView(renderTarget, app.getBackgroundColor());
	else
		context->ClearRenderTargetView(renderTarget, Colors::Black);
    // context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
	N3s3d::updateViews(viewport, renderTarget, depthStencil);

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
    width = 1024;
    height = 768;
}
void Game::getAppMessage(UINT message, WPARAM wParam, LPARAM lParam, HWND hwnd)
{
	switch (message)
	{
	case WM_ENTERMENULOOP:
		// Update the menu in the window
		updateMenu(GetMenu(hwnd), &app);
		//app.setMute(true);
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_LOAD:
		{
			string path = getFilePathWithDialog();
			if (path != "")
				app.load(path);
			break;
		}
		case ID_FILE_UNLOAD:
		{
			app.unload();
		}
		case ID_NES_RESET:
		{
			app.reset();
			break;
		}
		case ID_NES_PAUSE:
		{
			app.togglePause();
			break;
		}
		case ID_3D_SAVE:
		{
			app.save();
			break;
		}
		case ID__SAVE_AS:
		{
			string path = getSavePathWithDialog();
			if (path != "")
				app.saveAs(path);
			break;
		}
		case ID_3D_LOAD:
		{
			// MessageBox(NULL, L"ahh", L"File Path", MB_OK);
			string path = getFilePathWithDialog();
			if (path != "")
				app.loadGameData(path, false);
			break;
		}
		case ID_CONFIG_MUTEAUDIO:
			app.toggleMute();
			break;
		case ID_NES_RESETREGISTEROVERRIDES:
			N3sConfig::resetRegisterOverrides();
			break;
		default:
			// Assume this is a registry thing, then
			updateNesRegistry(LOWORD(wParam));
			break;
		}
		case WM_KEYDOWN:
		{
			bool previouslyDown = (int)lParam >> 30 & 1;
			if (!previouslyDown)
				app.recieveKeyInput(wParam, true);
			break;
		}
		case WM_KEYUP:
			app.recieveKeyInput(wParam, false);
			break;
		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_MENU)
				int test = 0;
			bool previouslyDown = (int)lParam >> 30 & 1;
			if (!previouslyDown)
				app.recieveKeyInput(wParam, true);
			break;
		}
		case WM_SYSKEYUP:
			app.recieveKeyInput(wParam, false);
			break;
		case WM_LBUTTONDOWN:
			app.recieveMouseInput(left_mouse, true);
			break;
		case WM_LBUTTONUP:
			app.recieveMouseInput(left_mouse, false);
			break;
		case WM_MBUTTONDOWN:
			app.recieveMouseInput(middle_mouse, true);
			break;
		case WM_MBUTTONUP:
			app.recieveMouseInput(middle_mouse, false);
			break;
		case WM_RBUTTONDOWN:
			app.recieveMouseInput(right_mouse, true);
			break;
		case WM_RBUTTONUP:
			app.recieveMouseInput(right_mouse, false);
			break;
		case WM_MOUSEMOVE:
			// Send X & Y
			app.recieveMouseMovement(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_MOUSEWHEEL:
			app.recieveMouseScroll(GET_WHEEL_DELTA_WPARAM(wParam));
			break;
	}
	case WM_ENTERSIZEMOVE:
	{
		app.setMute(true);
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		app.setMute(false);
		break;
	}
	}
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

string getFilePathWithDialog()
{
	string path = "";
	HRESULT hr = CoInitializeEx(NULL, COINITBASE_MULTITHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
						char buffer[500];
						wcstombs(buffer, pszFilePath, 500);
						path = buffer;
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return path;
}

string getSavePathWithDialog()
{
	string path = "";
	HRESULT hr = CoInitializeEx(NULL, COINITBASE_MULTITHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog *pFileSave;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
		if (SUCCEEDED(hr))
		{
			// Set default extension
			hr = pFileSave->SetDefaultExtension(L"n3s");
			if (SUCCEEDED(hr))
			{
				// Show the Open dialog box.
				hr = pFileSave->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem *pItem;
					hr = pFileSave->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{
							//MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
							char buffer[500];
							wcstombs(buffer, pszFilePath, 500);
							path = buffer;
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileSave->Release();
			}
		}
		CoUninitialize();
	}
	return path;
}
