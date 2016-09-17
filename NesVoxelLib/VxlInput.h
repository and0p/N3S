#pragma once

#include <Xinput.h>

#pragma comment(lib, "XInput.lib") // XInput library

enum nesButtons {

};

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

class KeyboardState
{
public:
	KeyboardState();
	bool keyStates[256];
	void setDown(int key);
	void setUp(int key);
};

class InputState
{
public:
	InputState();
	bool connected[2];
	ControllerState gamePads[2];
	KeyboardState keyboardState;
	void checkGamePads();
	void refreshInput();
};