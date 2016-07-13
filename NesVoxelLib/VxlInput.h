#pragma once

#include <Xinput.h>

#pragma comment(lib, "XInput.lib") // Library containing necessary 360

enum buttonNames { xdUp, xdDown, xdLeft, xdRight, xa, xb, xx, xy, xlb, xrb, xselect, xstart, xlClick, xrClick };

class ControllerState
{
public:
	ControllerState();
	ControllerState(XINPUT_GAMEPAD gamepad);
	bool buttonStates[14];
};

class InputState
{
public:
	InputState();
	bool connected[2];
	ControllerState gamePads[2];
	void refreshInput();
};