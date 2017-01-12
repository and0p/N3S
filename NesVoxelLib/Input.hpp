#pragma once

#include <Xinput.h>
#include <vector>

#pragma comment(lib, "XInput.lib") // XInput library

using namespace std;

const int totalKeys = 256;
const int xboxButtonCount = 14;
const int xboxAxisCount = 6;

enum inputFunctions {
	nes_p1_a, nes_p1_b, nes_p1_up, nes_p1_left, nes_p1_down, nes_p1_right, nes_p1_start, nes_p1_select,
	nes_p2_a, nes_p2_b, nes_p2_up, nes_p2_left, nes_p2_down, nes_p2_right, nes_p2_start, nes_p2_select,
	emu_pause, emu_reset, 
	cam_left, cam_right, cam_up, cam_down, cam_pan_in, cam_pan_out,
	LAST
};

enum device { keyboardMouse, gamepad1, gamepad2 };

enum buttonNames { bdUp, bdDown, bdLeft, bdRight, ba, bb, bx, by, blb, brb, bselect, bstart, blClick, brClick, LAST };
enum axisNames { leftXPos, leftXNeg, leftYPos, leftYNeg, rightXPos, rightXNeg, rightYPos, rightYNeg, lTrigger, rTrigger, LAST };

class Input
{
public:
	Input() {}
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
	DigitalInput() {}
	void update();
	float getValue();
};

class AnalogInput : Input
{
public:
	AnalogInput(bool negative) : negative(negative) {}
	float value = 0.0f;
	float deadzone = 0.1f;
	bool negative = false;
	void update();
	float getValue();
};

class InputDevice
{
public:
	InputDevice() {}
	// virtual DigitalInput getDigitalInput(int inputNumber) = 0;
	// virtual AnalogInput getAnalogInput(int inputNumber) = 0;
};

class KeyboardMouseDevice : InputDevice
{
public:
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
public:
	GamepadDevice();
	bool connected = false;
	shared_ptr<DigitalInput> buttons[xboxButtonCount];
	shared_ptr<AnalogInput> analogInputs[xboxAxisCount];
	void update(bool connected, XINPUT_GAMEPAD gamepad);
};

class Binding
{
public:
	Binding(shared_ptr<Input> input) : input(input), ctrl(false), alt(false), shift(false) {}
	Binding(shared_ptr<Input> input, bool ctrl, bool alt, bool shift) : input(input), ctrl(ctrl), alt(alt), shift(shift) {}
	shared_ptr<Input> input;
	bool ctrl = false;
	bool alt = false;
	bool shift = false;
};

class InputFunction
{
public:
	vector<Binding> bindings;
	bool active;
	bool activatedThisFrame;
	int framesActive;
	float value;
	void update();
};

class InputState
{
public:
	InputState();
	static shared_ptr<KeyboardMouseDevice> keyboardMouse;
	static shared_ptr<GamepadDevice> gamepads[2];
	static InputFunction functions[inputFunctions::LAST];
	static void checkGamePads();
	static void refreshInput();
	static void sendNesInput();
	static void createBindings();
};