#pragma once
#include "pch.h"
#include <Input.hpp>

class InputWindow {
public:
	static bool CALLBACK InputWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static string commitChanges();
};