#pragma once

#include <Xinput.h>
#include <vector>
#include <set>
#include <map>
#include "InputMapping.hpp"
#include "json.hpp"

#pragma comment(lib, "XInput.lib") // XInput library

using namespace std;
using json = nlohmann::json;

const int totalKeys = 256;
enum MouseState { inactive, clicked, down, pressed, dragging };
enum MouseButtons { left_mouse, right_mouse, middle_mouse, MOUSEBUTTONCOUNT };

const int keyNameCount = 108;

class Input
{
public:
	Input() {}
	bool active = false;
	int framesActive = 0;
	bool activatedThisFrame = false;
	string name = "";
	// Jesus Christ, C++ polymorphism is the worst
	void setActive(bool a) { active = a; }
	void setFramesActive(int a) { framesActive = a; }
	void setActivatedThisFrame(bool a) { activatedThisFrame = a; }
	void setName(string n);
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

class AnalogInput : public Input
{
public:
	AnalogInput(bool negative) : negative(negative) {}
	float value = 0.0f;
	float deadzone = 0.1f;
	bool negative = false;
	void update();
	float getValue();
};

class MouseButton
{
public:
	MouseState state = inactive;
	int x = 0;
	int y = 0;
	int actionXStart = -1;
	int actionYStart = -1;
	void update(int x, int y);
	bool down = false;
private:
	int framesActive = 0;
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
	float previousMouseX;
	float previousMouseY;
	float mouseDeltaX;
	float mouseDeltaY;
	shared_ptr<DigitalInput> keys[totalKeys];
	MouseButton mouseButtons[MouseButtons::MOUSEBUTTONCOUNT];
	int wheelDelta = 0;
	float calculatedWheelDelta = 0;
	void setDown(int key);
	void setUp(int key);
	void update();
	bool hasMouseMoved();
	string getInputNameThisFrame();
};

class GamepadDevice : InputDevice
{
public:
	GamepadDevice();
	bool connected = false;
	shared_ptr<DigitalInput> buttons[BUTTONCOUNT];
	shared_ptr<AnalogInput> analogInputs[AXISCOUNT];
	void update(bool connected, XINPUT_GAMEPAD gamepad);
	string getInputNameThisFrame(int gamepadNumber);
};

class Binding
{
public:
	Binding(shared_ptr<Input> input) : input(input), ctrl(false), alt(false), shift(false) {}
	Binding(shared_ptr<Input> input, bool ctrl, bool alt, bool shift) : input(input), ctrl(ctrl), alt(alt), shift(shift) {}
	//bool active();
	bool activatedThisFrame();
	shared_ptr<Input> input;
	bool ctrl = false;
	bool alt = false;
	bool shift = false;
};

class InputFunction
{
public:
	inline InputFunction() {}
	vector<Binding> bindings;
	bool active;
	bool activatedThisFrame;
	int framesActive;
	float value;
	void update();
};

class InputConfig
{
public:
	InputConfig();
	bool load(json j);
	string verify();
	json toJSON();
	map<string, string[2]> bindings;
};

class InputState
{
public:
	InputState();
	static shared_ptr<KeyboardMouseDevice> keyboardMouse;
	static shared_ptr<GamepadDevice> gamepads[2];
	static shared_ptr<InputFunction> functions[FUNCTION_COUNT];
	static map<int, string> keyNameMap;
	static void checkGamePads();
	static void refreshInput();
	static void sendNesInput();
	static void createBindings();
	static InputConfig getInputConfig();
	static void setDefaultInputConfig();
	static string applyInputConfig(InputConfig c);
	static map<string, shared_ptr<Input>> inputsByName;
	static map<string, shared_ptr<InputFunction>> functionsByName;
	static set<string> configurableFunctions;
	static set<string> bindableInputs;
	static vector<string> orderedBindableInputs;
private:
	static InputConfig inputConfig;
};