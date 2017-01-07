#pragma once

#include <Xinput.h>

#pragma comment(lib, "XInput.lib") // XInput library

using namespace std;

const int totalKeys = 256;
const int xboxButtonCount = 14;
const int xboxAxisCount = 6;

enum inputFunctions {
	nes_p1_a, nes_p1_b, nes_p1_up, nes_p1_left, nes_p1_down, nes_p1_right, nes_p1_start, nes_p1_select,
	nes_p2_a, nes_p2_b, nes_p2_up, nes_p2_left, nes_p2_down, nes_p2_right, nes_p2_start, nes_p1_select,
	emu_pause, emu_reset, LAST
};

enum device { keyboardMouse, gamepad1, gamepad2 };

enum buttonNames { bdUp, bdDown, bdLeft, bdRight, ba, bb, bx, by, blb, brb, bselect, bstart, blClick, brClick };
enum axisNames { leftX, leftY, rightX, rightY, lTrigger, rTrigger };

class Input
{
public:
	bool active = false;
	int framesActive = 0;
	bool activatedThisFrame = false;
	// Jesus Christ, C++ polymorphism is the worst
	void setActive(bool a) { active = a; }
	void setFramesActive(int a) { framesActive = a; }
	void setActivatedThisFrame(bool a) { activatedThisFrame = a; }
	virtual float getValue() = 0;
	virtual void update() = 0;
};

class DigitalInput : public Input
{
public:
	void update();
	float getValue();
};

class AnalogInput : Input
{
public:
	float value = 0.0f;
	float deadzone = 0.1f;
	void update();
	float getValue();
};

class InputFunction
{
	vector<Binding> bindings;
	bool active;
	int activeTime;
};

class InputDevice
{
	virtual DigitalInput getDigitalInput(int inputNumber) = 0;
	virtual AnalogInput getAnalogInput(int inputNumber) = 0;
};

class KeyboardMouseDevice : InputDevice
{
	KeyboardMouseDevice();
	float mouseX;
	float mouseY;
	shared_ptr<DigitalInput> keys[totalKeys];
	void setDown(int key);
	void setUp(int key);
	void update();
};

class GamepadDevice : InputDevice
{
	bool connected = false;
	shared_ptr<DigitalInput> buttons[xboxButtonCount];
	shared_ptr<AnalogInput> analogInputs[xboxAxisCount];
	void update(XINPUT_GAMEPAD gamepad);
};

class Binding
{
	shared_ptr<Input> input;
	bool ctrl = false;
	bool alt = false;
	bool shift = false;
};

class InputState
{
public:
	InputState();
	shared_ptr<KeyboardMouseDevice> keyboardMouse;
	shared_ptr<GamepadDevice> gamepads[2];
	shared_ptr<Binding> bindings[inputFunctions::LAST];
	bool connected[2];
	
	void checkGamePads();
	void refreshInput();
};