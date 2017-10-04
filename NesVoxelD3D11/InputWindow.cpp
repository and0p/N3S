#include "pch.h"
#include "InputWindow.h"
#include "resource.h"

void InputWindow::initialize()
{
	InputConfig ic;
	string buttonName = "ABCD";
	SendMessage((HWND)IDC_COMBO_P1_A_1, CB_ADDSTRING, 0, (LPARAM)buttonName.c_str());
}

bool CALLBACK InputWindow::InputWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			commitChanges();
			EndDialog(hDlg, 0);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
		}
	}
	return false;
}

bool InputWindow::commitChanges()
{
	return true;
}