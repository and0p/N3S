#include "pch.h"
#include "InputWindow.h"
#include <Input.hpp>
#include "resource.h"
#include <unordered_map>
#include "atlstr.h"
#include "N3sConfig.hpp"

using namespace std;

struct NameAndBindingNumber {
	string name;
	int bindingNumber;
};

HWND handle;
unordered_map<int, NameAndBindingNumber > dropdownMap;
unordered_map<string, int> inputNameOrder;				// Order of items in drop down, for selecting by index
bool previouslyPopulated;

void initDialog(HWND hDlg)
{
	handle = hDlg;
	// Get input config from game
	InputConfig config = InputState::getInputConfig();
	// Set the error to be empty
	//string empty = "";
	//SendDlgItemMessage(hDlg, IDC_ERRORMSG, WM_SETTEXT, NULL, (LPARAM)empty.c_str());
	// Init all options for dropdowns, etc, if haven't run yet
	if (!previouslyPopulated)
	{
		dropdownMap[IDC_COMBO_P1_A_1] = { "NES P1 A", 1 };
		dropdownMap[IDC_COMBO_P1_B_1] = { "NES P1 B", 1 };
		dropdownMap[IDC_COMBO_P1_UP_1] = { "NES P1 UP", 1 };
		dropdownMap[IDC_COMBO_P1_LEFT_1] = { "NES P1 LEFT", 1 };
		dropdownMap[IDC_COMBO_P1_DOWN_1] = { "NES P1 DOWN", 1 };
		dropdownMap[IDC_COMBO_P1_RIGHT_1] = { "NES P1 RIGHT", 1 };
		dropdownMap[IDC_COMBO_P1_START_1] = { "NES P1 START", 1 };
		dropdownMap[IDC_COMBO_P1_SELECT_1] = { "NES P1 SELECT", 1 };
		dropdownMap[IDC_COMBO_P1_A_2] = { "NES P1 A", 2 };
		dropdownMap[IDC_COMBO_P1_B_2] = { "NES P1 B", 2 };
		dropdownMap[IDC_COMBO_P1_UP_2] = { "NES P1 UP", 2 };
		dropdownMap[IDC_COMBO_P1_LEFT_2] = { "NES P1 LEFT", 2 };
		dropdownMap[IDC_COMBO_P1_DOWN_2] = { "NES P1 DOWN", 2 };
		dropdownMap[IDC_COMBO_P1_RIGHT_2] = { "NES P1 RIGHT", 2 };
		dropdownMap[IDC_COMBO_P1_START_2] = { "NES P1 START", 2 };
		dropdownMap[IDC_COMBO_P1_SELECT_2] = { "NES P1 SELECT", 2 };
		dropdownMap[IDC_COMBO_P2_A_1] = { "NES P2 A", 1 };
		dropdownMap[IDC_COMBO_P2_B_1] = { "NES P2 B", 1 };
		dropdownMap[IDC_COMBO_P2_UP_1] = { "NES P2 UP", 1 };
		dropdownMap[IDC_COMBO_P2_LEFT_1] = { "NES P2 LEFT", 1 };
		dropdownMap[IDC_COMBO_P2_DOWN_1] = { "NES P2 DOWN", 1 };
		dropdownMap[IDC_COMBO_P2_RIGHT_1] = { "NES P2 RIGHT", 1 };
		dropdownMap[IDC_COMBO_P2_START_1] = { "NES P2 START", 1 };
		dropdownMap[IDC_COMBO_P2_SELECT_1] = { "NES P2 SELECT", 1 };
		dropdownMap[IDC_COMBO_P2_A_2] = { "NES P2 A", 2 };
		dropdownMap[IDC_COMBO_P2_B_2] = { "NES P2 B", 2 };
		dropdownMap[IDC_COMBO_P2_UP_2] = { "NES P2 UP", 2 };
		dropdownMap[IDC_COMBO_P2_LEFT_2] = { "NES P2 LEFT", 2 };
		dropdownMap[IDC_COMBO_P2_DOWN_2] = { "NES P2 DOWN", 2 };
		dropdownMap[IDC_COMBO_P2_RIGHT_2] = { "NES P2 RIGHT", 2 };
		dropdownMap[IDC_COMBO_P2_START_2] = { "NES P2 START", 2 };
		dropdownMap[IDC_COMBO_P2_SELECT_2] = { "NES P2 SELECT", 2 };
		// Get indexes of all binding options, for setting dropdowns by index #
		for (int i = 0; i < InputState::orderedBindableInputs.size(); i++)
		{
			inputNameOrder[InputState::orderedBindableInputs[i]] = i;
		}
		// Add all input option to all comboboxes
		for each(auto kv in dropdownMap)
		{
			for each(string s in InputState::orderedBindableInputs)
			{
				SendDlgItemMessage(hDlg, kv.first, CB_ADDSTRING, 0, (LPARAM)std::wstring(s.begin(), s.end()).c_str());
			}
		}
	}
	// Set the dropdowns to their current configurations
	for each(auto kv in dropdownMap)
	{
		// See if its bindings are specified in the app's configuration
		if (config.bindings.count(kv.second.name) > 0)
		{
			string b1 = config.bindings[kv.second.name][kv.second.bindingNumber - 1];
			// See if this binding is not empty
			if (b1 != "")
			{
				// See if we have an index number for this input ie "JOY1_R_TRIGGER", otherwise ignore
				// (This lets us seek the string much faster)
				if (inputNameOrder.count(b1) > 0)
				{
					// Set the selected index of the dropdown
					SendDlgItemMessage(hDlg, kv.first, CB_SELECTSTRING, inputNameOrder[b1] - 1, (LPARAM)std::wstring(b1.begin(), b1.end()).c_str());
				}
			}
		}
	}
}

bool CALLBACK InputWindow::InputWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	string result = "";
	switch (message)
	{
	case WM_INITDIALOG:
		initDialog(hDlg);
		break;
	case WM_DESTROY:
		EndDialog(hDlg, 0);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			result = commitChanges();
			if (result == "")
			{
				N3sConfig::save();
				EndDialog(hDlg, 0);
			}
			else
				//const int lol = MessageBox(handle, (LPCWSTR)std::wstring(result.begin(), result.end()).c_str(), NULL, MB_OK | MB_ICONWARNING);
				SendDlgItemMessage(hDlg, IDC_ERRORMSG, WM_SETTEXT, NULL, (LPARAM)std::wstring(result.begin(), result.end()).c_str());
;			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;
	}
	return DefWindowProc(hDlg, message, wParam, lParam);
}

string InputWindow::commitChanges()
{
	// Apply specified bindings to a new InputConfig
	InputConfig ic;
	for each (auto kv in dropdownMap)
	{
		LPWSTR s;
		GetDlgItemText(handle, kv.first, s, 256);
		string converted = CW2A(s);
		// See if the string is not empty
		if (converted != "")
			ic.bindings[kv.second.name][kv.second.bindingNumber - 1] = converted;
	}
	// Send this input config to the program
	return InputState::applyInputConfig(ic);
}