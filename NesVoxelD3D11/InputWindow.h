#pragma once
#include "pch.h"
#include <Input.hpp>

class InputWindow {
public:
	static void initialize(InputConfig ic);
	static bool CALLBACK InputWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static bool commitChanges();
};