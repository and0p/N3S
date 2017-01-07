#pragma once

#include <Xinput.h>

#pragma comment(lib, "XInput.lib") // XInput library

using namespace std;

const int totalKeys = 100;
const int xboxButtonCount = 14;
const int xboxAxisCount = 6;

enum nesButtons {

};

enum inputFunctions {
	nes_p1_a, nes_p1_b, nes_p1_up, nes_p1_left, nes_p1_down, nes_p1_right, nes_p1_start, nes_p1_select,
	nes_p2_a, nes_p2_b, nes_p2_up, nes_p2_left, nes_p2_down, nes_p2_right, nes_p2_start, nes_p1_select,
	emu_pause, emu_reset
};

enum device { keyboardMouse, gamepad1, gamepad2 };

enum buttonNames { bdUp, bdDown, bdLeft, bdRight, ba, bb, bx, by, blb, brb, bselect, bstart, blClick, brClick };
enum triggerNames { lTrigger, rTrigger };
enum stickNames { lStick, rStick };

class Input
{
public:
	int framesActive;
	virtual bool isActive() = 0;
	virtual bool activatedThisFrame() = 0;
	virtual float getValue() = 0;
	virtual void update() = 0;
};

class DigitalInput : virtual public Input
{
	void update();
	int device;
	int button;
	int framesPressed;
};

class AnalogInput : Input
{
	int device;
	int axis;
	void update();
};

class Binding
{
	shared_ptr<Input> input;
	bool ctrl = false;
	bool alt = false;
	bool shift = false;
};

class InputFunction
{
	vector<Binding> bindings;
	bool active;
	int activeTime;
};

class InputDevice
{
	virtual void update() = 0;
	virtual DigitalInput getDigitalInput(int inputNumber) = 0;
	virtual AnalogInput getAnalogInput(int inputNumber) = 0;
};

class KeyboardMouseDevice : InputDevice
{
	float mouseX;
	float mouseY;
	shared_ptr<DigitalInput> keys[totalKeys];
};

class GamepadDevice : InputDevice
{
	shared_ptr<DigitalInput> buttons[xboxButtonCount];
	shared_ptr<AnalogInput> analogInputs[xboxAxisCount];
};

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
	static ControllerState gamePads[2];
	static KeyboardState keyboardState;
	void checkGamePads();
	void refreshInput();
};