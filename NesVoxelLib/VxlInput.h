#pragma once

#include <Xinput.h>

#pragma comment(lib, "XInput.lib") // XInput library

enum buttonNames { bdUp, bdDown, bdLeft, bdRight, ba, bb, bx, by, blb, brb, bselect, bstart, blClick, brClick };
enum triggerNames { lTrigger, rTrigger };
enum stickNames { lStick, rStick };

struct AnalogStickState
{
	float x;
	float y;
};

class ControllerState
{
public:
	ControllerState();
	ControllerState(XINPUT_GAMEPAD gamepad);
	bool buttonStates[14];
	AnalogStickState analogStickStates[2];
	float triggerStates[2];
};

class InputState
{
public:
	InputState();
	bool connected[2];
	ControllerState gamePads[2];
	void refreshInput();
};