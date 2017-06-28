//
// Main.cpp
//

#include "pch.h"
#include "Game.h"
#include "resource.h"


using namespace DirectX;

namespace
{
    std::unique_ptr<Game> g_game;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool readyToExit = false;
HMENU menu;
bool menuShown = true;

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

    g_game = std::make_unique<Game>();

    // Register class and create window
    {
        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(wcex.hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
        wcex.lpszClassName = L"NesVoxelD3D11WindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, L"IDI_ICON");
        if (!RegisterClassEx(&wcex))
            return 1;

        // Create window
        int w, h;
        g_game->GetDefaultSize(w, h);

        RECT rc;
        rc.top = 0;
        rc.left = 0;
        rc.right = static_cast<LONG>(w); 
        rc.bottom = static_cast<LONG>(h);

        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowEx(0, L"NesVoxelD3D11WindowClass", L"N3S", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        // TODO: Change to CreateWindowEx(WS_EX_TOPMOST, L"NesVoxelD3D11WindowClass", L"NesVoxelD3D11", WS_POPUP,
        // to default to fullscreen.

		// Get menu handle from window we just made
		menu = GetMenu(hwnd);

        if (!hwnd)
            return 1;

        ShowWindow(hwnd, nCmdShow);
        // TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_game.get()) );

        GetClientRect(hwnd, &rc);

        g_game->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);
    }

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message && !readyToExit)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            g_game->Tick();
        }
    }

    g_game.reset();

    CoUninitialize();

    return (int) msg.wParam;
}

// Windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	static bool s_in_sizemove = false;
	static bool s_in_suspend = false;
	static bool s_minimized = false;
	static bool s_fullscreen = false;
	// TODO: Set s_fullscreen to true if defaulting to fullscreen.

	auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!s_minimized)
			{
				s_minimized = true;
				if (!s_in_suspend && game)
					game->OnSuspending();
				s_in_suspend = true;
			}
		}
		else if (s_minimized)
		{
			s_minimized = false;
			if (s_in_suspend && game)
				game->OnResuming();
			s_in_suspend = false;
		}
		else if (!s_in_sizemove && game)
		{
			game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_ENTERSIZEMOVE:
		s_in_sizemove = true;
		break;

	case WM_EXITSIZEMOVE:
		s_in_sizemove = false;
		if (game)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_GETMINMAXINFO:
	{
		auto info = reinterpret_cast<MINMAXINFO*>(lParam);
		info->ptMinTrackSize.x = 320;
		info->ptMinTrackSize.y = 200;
	}
	break;

	case WM_ACTIVATEAPP:
		if (game)
		{
			if (wParam)
			{
				game->OnActivated();
			}
			else
			{
				game->OnDeactivated();
			}
		}
		break;

	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
			if (!s_in_suspend && game)
				game->OnSuspending();
			s_in_suspend = true;
			return true;

		case PBT_APMRESUMESUSPEND:
			if (!s_minimized)
			{
				if (s_in_suspend && game)
					game->OnResuming();
				s_in_suspend = false;
			}
			return true;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SYSKEYDOWN:
		// See if we care about this system key, if not pass it to game
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			// Implements the classic ALT+ENTER fullscreen toggle
			if (s_fullscreen)
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

				int width = 800;
				int height = 600;
				if (game)
					game->GetDefaultSize(width, height);

				ShowWindow(hWnd, SW_SHOWNORMAL);

				SetMenu(hWnd, menu);
				menuShown = true;

				SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, 0);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
				SetMenu(hWnd, NULL);
				menuShown = false;

				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			}

			s_fullscreen = !s_fullscreen;
		}
		else
		{
			g_game->getAppMessage(message, wParam, lParam, hWnd);
		}
		//else if ((lParam & 0x60000000) == 0x20000000 && s_fullscreen)
		//{
		//	if (menuShown)
		//	{
		//		SetMenu(hWnd, NULL);
		//		menuShown = false;
		//	}
		//	else
		//	{
		//		SetMenu(hWnd, menu);
		//		menuShown = true;
		//	}

		//}
		break;

	case WM_KEYDOWN:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;

	case WM_KEYUP:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;

	case WM_SYSKEYUP:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;

	case WM_LBUTTONDOWN:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;
	case WM_LBUTTONUP:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;
	case WM_MBUTTONDOWN:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;
	case WM_MBUTTONUP:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;
	case WM_RBUTTONDOWN:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;
	case WM_RBUTTONUP:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;
	case WM_MOUSEMOVE:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;

	case WM_MOUSEWHEEL:
		g_game->getAppMessage(message, wParam, lParam, hWnd);
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			readyToExit = true;
			break;
		default:
			g_game->getAppMessage(message, wParam, lParam, hWnd);
			break;
		}
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
	
}