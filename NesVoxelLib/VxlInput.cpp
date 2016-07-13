#include "stdafx.h"
#include "VxlInput.h"

ControllerState::ControllerState()
{
}

ControllerState::ControllerState(XINPUT_GAMEPAD gamepad)
{
	// Read buttons
	buttonStates[xa] = ((gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
	buttonStates[xb] = ((gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
	buttonStates[xx] = ((gamepad.wButtons & XINPUT_GAMEPAD_X) != 0);
	buttonStates[xy] = ((gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0);
	buttonStates[xstart] = ((gamepad.wButtons & XINPUT_GAMEPAD_START) != 0);
	buttonStates[xselect] = ((gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0);
	buttonStates[xdLeft] = ((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
	buttonStates[xdRight] = ((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
	buttonStates[xdUp] = ((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
	buttonStates[xdDown] = ((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
}

InputState::InputState()
{
}

void InputState::refreshInput()
{
	XINPUT_STATE state;
	//ZeroMemory(&state, sizeof(XINPUT_STATE));

	// See if either controller is connected and grab input state if so
	if (XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		connected[0] = true;
		gamePads[0] = ControllerState(state.Gamepad);
	}
	else
	{
		connected[0] = false;
	}
	if (XInputGetState(1, &state) == ERROR_SUCCESS)
	{
		connected[1] = true;
		gamePads[1] = ControllerState(state.Gamepad);
	}
	else
	{
		connected[1] = false;
	}
	
}
