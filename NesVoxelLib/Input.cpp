#include "stdafx.h"
#include "Input.hpp"
#include "NesEmulator.hpp"
#include "libretro.h"

shared_ptr<KeyboardMouseDevice> InputState::keyboardMouse;
shared_ptr<GamepadDevice> InputState::gamepads[2];
shared_ptr<InputFunction> InputState::functions[FUNCTION_COUNT];
map<string, shared_ptr<Input>> InputState::inputsByName;
InputConfig InputState::inputConfig;
map<string, shared_ptr<InputFunction>> InputState::functionsByName;
set<string> InputState::configurableFunctions;
set<string> InputState::bindableInputs;
vector<string> InputState::orderedBindableInputs;
map<int, string> InputState::keyNameMap;

void DigitalInput::update()
{
	if (active)
	{
		if (framesActive > 0)
			setActivatedThisFrame(false);
		else
			setActivatedThisFrame(true);
		setFramesActive(framesActive + 1);
	}
	else
	{
		setActivatedThisFrame(false);
		setFramesActive(0);
	}
}

float DigitalInput::getValue()
{
	if (active)
		return 1.0f;
	else
		return 0.0f;
}

void AnalogInput::update()
{
	// Flip value if negative
	if (negative)
	{
		value *= -1;
	}
	// Set the value
	if (value >= deadzone)
	{
		setActive(true);
		if (framesActive > 0)
			setActivatedThisFrame(false);
		else
			setActivatedThisFrame(true);
		framesActive++;
		// Adjust value based on deadzone area
		value -= deadzone;
		value = (float)(value / (1.0f - deadzone));
	}
	else
	{
		setActivatedThisFrame(false);
		framesActive = 0;
		setActive(false);
		value = 0;
	}
}

float AnalogInput::getValue()
{
	return value;
}

void MouseButton::update(int newX, int newY)
{
	x = newX;
	y = newY;
	if (down)
	{
		if (framesActive == 0)
		{
			state = clicked;
			actionXStart = x;
			actionYStart = y;
		}
		else if (framesActive > 0)
		{
			// Don't change to/from dragging state if we're already dragging
			if (state != dragging)
			{
				// Get distance between possible action start and this X/Y
				int xDelta = abs(actionXStart - x);
				int yDelta = abs(actionYStart - y);
				// See if X or Y are further than 0 pixels away
				if (xDelta > 1 || yDelta > 1)
				{
					state = dragging;
				}
				else
					state = MouseState::down;
			}
		}
		framesActive++;
	}
	else
	{
		// If we're releasing near the click start it counts as "pressed" for buttons
		if (framesActive > 0 && framesActive < 200)
		{
			// Get distance between possible action start and this X/Y
			int xDelta = abs(actionXStart - x);
			int yDelta = abs(actionYStart - y);
			if (xDelta < 1 && yDelta < 1)
				state = pressed;
		}
		else
		{
			state = inactive;
		}
		framesActive = 0;
	}
}

KeyboardMouseDevice::KeyboardMouseDevice()
{
	for (int i = 0; i < totalKeys; i++)
	{
		keys[i] = make_shared<DigitalInput>();
	}
}

void KeyboardMouseDevice::setDown(int key)
{
	if (key >= 0 && key < totalKeys)
	{
		keys[key]->setActive(true);
	}
}

void KeyboardMouseDevice::setUp(int key)
{
	keys[key]->setActive(false);
}

void KeyboardMouseDevice::update()
{
	// Get deltas and reset previous positions
	mouseDeltaX = mouseX - previousMouseX;
	mouseDeltaY = mouseY - previousMouseY;
	previousMouseX = mouseX;
	previousMouseY = mouseY;
	for (int i = 0; i < totalKeys; i++)
	{
		keys[i]->update();
	}
	for (int i = 0; i < MOUSEBUTTONCOUNT; i++)
	{
		mouseButtons[i].update(mouseX, mouseY);
	}
	calculatedWheelDelta = wheelDelta / 120;
	wheelDelta = 0;
}

bool KeyboardMouseDevice::hasMouseMoved()
{
	if (mouseDeltaX != 0 || mouseDeltaY != 0)
		return true;
	else
		return false;
}

// For configuration window, lets it know what button was pressed this frame
string KeyboardMouseDevice::getInputNameThisFrame()
{
	// Check if any keyboard keys have been activated this frame
	for (int i = 0; i < 256; i++)
	{
		// See if activated this frame
		if (keys[i]->activatedThisFrame)
		{
			// See if this is a bindable (or named) input (not ESC or anything...)
			if (InputState::keyNameMap.count(i) > 0)
			{
				return InputState::keyNameMap[i];
			}
		}
	}
	// Check if any bindable mouse buttons were pressed
		// Actually, since right and middle click are taken, I don't think there should be...
		// TODO: think about this later
	return "";
}

GamepadDevice::GamepadDevice()
{
	for (int i = 0; i < BUTTONCOUNT; i++)
	{
		buttons[i] = make_shared<DigitalInput>();
	}
	analogInputs[AX_L_STICK_RIGHT] = make_shared<AnalogInput>(false);
	analogInputs[AX_L_STICK_LEFT] = make_shared<AnalogInput>(true);
	analogInputs[AX_L_STICK_UP] = make_shared<AnalogInput>(false);
	analogInputs[AX_L_STICK_DOWN] = make_shared<AnalogInput>(true);
	analogInputs[AX_R_STICK_RIGHT] = make_shared<AnalogInput>(false);
	analogInputs[AX_R_STICK_LEFT] = make_shared<AnalogInput>(true);
	analogInputs[AX_R_STICK_UP] = make_shared<AnalogInput>(false);
	analogInputs[AX_R_STICK_DOWN] = make_shared<AnalogInput>(true);
	analogInputs[AX_L_TRIGGER] = make_shared<AnalogInput>(false);
	analogInputs[AX_R_TRIGGER] = make_shared<AnalogInput>(false);
}

void GamepadDevice::update(bool connected, XINPUT_GAMEPAD gamepad)
{
	if (connected)
	{
		// Read buttons
		buttons[BTN_A]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_A) != 0));
		buttons[BTN_B]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_B) != 0));
		buttons[BTN_X]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_X) != 0));
		buttons[BTN_Y]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0));
		buttons[BTN_START]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_START) != 0));
		buttons[BTN_SELECT]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0));
		buttons[DPAD_LEFT]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0));
		buttons[DPAD_RIGHT]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0));
		buttons[DPAD_UP]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0));
		buttons[DPAD_DOWN]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0));
		buttons[BTN_L_BUMPER]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0));
		buttons[BTN_R_BUMPER]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0));
		// Read analog sticks
		analogInputs[AX_L_STICK_LEFT]->value = gamepad.sThumbLX / 32767.0f;
		analogInputs[AX_L_STICK_RIGHT]->value = gamepad.sThumbLX / 32767.0f;
		analogInputs[AX_L_STICK_DOWN]->value = gamepad.sThumbLY / 32767.0f;
		analogInputs[AX_L_STICK_UP]->value = gamepad.sThumbLY / 32767.0f;
		analogInputs[AX_R_STICK_LEFT]->value = gamepad.sThumbRX / 32767.0f;
		analogInputs[AX_R_STICK_RIGHT]->value = gamepad.sThumbRX / 32767.0f;
		analogInputs[AX_R_STICK_DOWN]->value = gamepad.sThumbRY / 32767.0f;
		analogInputs[AX_R_STICK_UP]->value = gamepad.sThumbRY / 32767.0f; 
		analogInputs[AX_L_TRIGGER]->value = gamepad.bLeftTrigger / 255.0f;
		analogInputs[AX_R_TRIGGER]->value = gamepad.bRightTrigger / 255.0f;

		// Update all
		for (int i = 0; i < BUTTONCOUNT; i++)
		{
			buttons[i]->update();
		}
		for (int i = 0; i < AXISCOUNT; i++)
		{
			analogInputs[i]->update();
		}
	}
	else
	{
		for (int i = 0; i < BUTTONCOUNT; i++)
		{
			buttons[i]->setActive(false);
		}
		for (int i = 0; i < AXISCOUNT; i++)
		{
			analogInputs[i]->value = 0.0f;
		}
	}
}

string GamepadDevice::getInputNameThisFrame(int gamepadNumber)
{
	string gpString = to_string(gamepadNumber);
	if (buttons[BTN_A]->active) return "JOY" + gpString + "_BTN_A";
	else if (buttons[BTN_B]->active) return "JOY" + gpString + "_BTN_B";
	else if (buttons[BTN_X]->active) return "JOY" + gpString + "_BTN_X";
	else if (buttons[BTN_Y]->active) return "JOY" + gpString + "_BTN_Y";
	else if (buttons[BTN_START]->active) return "JOY" + gpString + "_BTN_START";
	else if (buttons[BTN_SELECT]->active) return "JOY" + gpString + "_BTN_SELECT";
	else if (buttons[DPAD_LEFT]->active) return "JOY" + gpString + "_DPAD_LEFT";
	else if (buttons[DPAD_RIGHT]->active) return "JOY" + gpString + "_DPAD_RIGHT";
	else if (buttons[DPAD_UP]->active) return "JOY" + gpString + "_DPAD_UP";
	else if (buttons[DPAD_DOWN]->active) return "JOY" + gpString + "_DPAD_DOWN";
	else if (buttons[BTN_L_BUMPER]->active) return "JOY" + gpString + "_L_BUMPER";
	else if (buttons[BTN_R_BUMPER]->active) return "JOY" + gpString + "_R_BUMPER";
	else if (buttons[BTN_L_CLICK]->active) return "JOY" + gpString + "_L_CLICK";
	else if (buttons[BTN_R_CLICK]->active) return "JOY" + gpString + "_R_CLICK";
	else if (analogInputs[AX_L_STICK_RIGHT]->active) return "JOY" + gpString + "_L_RIGHT";
	else if (analogInputs[AX_L_STICK_LEFT]->active) return "JOY" + gpString + "_L_LEFT";
	else if (analogInputs[AX_L_STICK_UP]->active) return "JOY" + gpString + "_L_UP";
	else if (analogInputs[AX_L_STICK_DOWN]->active) return "JOY" + gpString + "_L_DOWN";
	else if (analogInputs[AX_R_STICK_RIGHT]->active) return "JOY" + gpString + "_R_RIGHT";
	else if (analogInputs[AX_R_STICK_LEFT]->active) return "JOY" + gpString + "_R_LEFT";
	else if (analogInputs[AX_R_STICK_UP]->active) return "JOY" + gpString + "_R_UP";
	else if (analogInputs[AX_R_STICK_DOWN]->active) return "JOY" + gpString + "_R_DOWN";
	else if (analogInputs[AX_L_TRIGGER]->active) return "JOY" + gpString + "_L_TRIGGER";
	else if (analogInputs[AX_R_TRIGGER]->active) return "JOY" + gpString + "_R_TRIGGER";
	else return "";
}

InputState::InputState()
{
	keyboardMouse = make_shared<KeyboardMouseDevice>();
	gamepads[0] = make_shared<GamepadDevice>();
	gamepads[1] = make_shared<GamepadDevice>();
	createBindings();
	setDefaultInputConfig();
	json j = inputConfig.toJSON();
}

void InputState::checkGamePads()
{
	XINPUT_STATE state;
	// See if either controller is connected and grab input state if so
	if (XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		gamepads[0]->update(true, state.Gamepad);
	}
	else
	{
		gamepads[0]->update(false, state.Gamepad);
	}
	if (XInputGetState(1, &state) == ERROR_SUCCESS)
	{
		gamepads[1]->update(true, state.Gamepad);
	}
	else
	{
		gamepads[1]->update(false, state.Gamepad);
	}
}

void InputState::refreshInput()
{
	checkGamePads();
	keyboardMouse->update();
	// Update bindings and send input to NES
	for (int i = 0; i < FUNCTION_COUNT; i++)
	{
		functions[i]->update();
	}
	sendNesInput();
}

void InputState::sendNesInput()
{
	// Player 1
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_A] = functions[nes_p1_a]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_B] = functions[nes_p1_b]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_SELECT] = functions[nes_p1_select]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_START] = functions[nes_p1_start]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_UP] = functions[nes_p1_up]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_DOWN] = functions[nes_p1_down]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_LEFT] = functions[nes_p1_left]->active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_RIGHT] = functions[nes_p1_right]->active;
	// Player 2
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_A] = functions[nes_p2_a]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_B] = functions[nes_p2_b]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_SELECT] = functions[nes_p2_select]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_START] = functions[nes_p2_start]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_UP] = functions[nes_p2_up]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_DOWN] = functions[nes_p2_down]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_LEFT] = functions[nes_p2_left]->active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_RIGHT] = functions[nes_p2_right]->active;
}

void InputFunction::update()
{
	active = false;
	activatedThisFrame = false;
	framesActive = 0;
	value = 0;
	for (Binding b : bindings)
	{
		if (b.input->active)
			active = true;
		if (b.activatedThisFrame())
			activatedThisFrame = true;
		if (b.input->framesActive > framesActive)
			framesActive = b.input->framesActive;
		if (b.input->getValue() > value)
			value = b.input->getValue();
	}
}

void InputState::createBindings()
{
	// TODO: load from a proper config, build menu to edit

	// Map names of bindable inputs to digital / analog inputs
	for (int i = 0; i < inputCount; i++)
	{
		shared_ptr<Input> input;
		InputMapping m = inputMap[i];
		if (m.type == KEYBOARD)
		{
			// Map the input name to instance
			inputsByName[m.name] = keyboardMouse->keys[m.enumNumber];
			// Give the instance it's name
			keyboardMouse->keys[m.enumNumber]->setName(m.name);
			// If this input is bindable (not a critical system key) add to the set (and ordered vector)
			bindableInputs.insert(m.name);
			orderedBindableInputs.push_back(m.name);
		}
		else
		{
			if (m.analog)	// Switch based on analog input
			{
				// Map input name to instance
				inputsByName[m.name] = gamepads[m.deviceNumber]->analogInputs[m.enumNumber];
				// Give the instance it's name
				gamepads[m.deviceNumber]->analogInputs[m.enumNumber]->setName(m.name);
			}
			else
			{
				// Map input name to instance
				inputsByName[m.name] = gamepads[m.deviceNumber]->buttons[m.enumNumber];
				// Give the instance it's name
				gamepads[m.deviceNumber]->buttons[m.enumNumber]->setName(m.name);
			}
			// If this input is bindable (not a critical system key) add to the set (and ordered vector)
			bindableInputs.insert(m.name);
			orderedBindableInputs.push_back(m.name);
		}
	}

	// Set up functions
	for (int i = 0; i < FUNCTION_COUNT; i++)
	{
		// Create the function
		functions[i] = make_shared<InputFunction>();
		FunctionMapping f = functionMap[i];
		// Map function name to instance
		functionsByName[f.name] = functions[f.function];
		// Add to list of configurable if appropriate, for security
		if (f.configurable)
			configurableFunctions.insert(f.name);
	}
}


InputConfig InputState::getInputConfig()
{
	return inputConfig;
}

void InputState::setDefaultInputConfig()
{
	// Load default bindings
	for (int i = 0; i < FUNCTION_COUNT; i++)
	{
		FunctionMapping m = functionMap[i];
		// Grab associated function
		shared_ptr<InputFunction> f = functions[m.function];
		// Apply default bindings
		if (m.defaultBinding1 != "")
		{
			shared_ptr<Input> firstDefault = inputsByName[m.defaultBinding1];
			f->bindings.push_back({ firstDefault });
		}
		if (m.defaultBinding2 != "")
		{
			shared_ptr<Input> secondDefault = inputsByName[m.defaultBinding2];
			f->bindings.push_back({ secondDefault });
		}
		// If this is configurable, add to input configuration
		if (m.configurable)
		{
			// Am I awful at this or is C++ just obnoxious about doing simple things?
			if(m.defaultBinding1 != "")
				inputConfig.bindings[m.name][0] = m.defaultBinding1;
			if(m.defaultBinding2 != "")
				inputConfig.bindings[m.name][1] = m.defaultBinding2;
		}
	}
}

string InputState::applyInputConfig(InputConfig c)
{
	// Make sure input config is not malformed
	string msg = c.verify();
	// Apply the config if there is no error message
	if (msg == "")
	{
		for each (auto kv in c.bindings)
		{
			// Grab the function and clear any current bindings
			shared_ptr<InputFunction> f = functionsByName[kv.first];
			f->bindings.clear();
			// Push both new bindings
			if(kv.second[0] != "")
				f->bindings.push_back(inputsByName[kv.second[0]]);
			if (kv.second[1] != "")
				f->bindings.push_back(inputsByName[kv.second[1]]);
		}
		// Save the passed config
		inputConfig = c;
	}
	return msg;
}

bool Binding::activatedThisFrame()
{
	if (input->activatedThisFrame)
	{
		if (ctrl) {
			if (InputState::keyboardMouse->keys[VK_CONTROL]->active)
				return true;
			else
				return false;
		}
		else
			return true;
	}
	else
	{
		return false;
	}
}

InputConfig::InputConfig()
{
}

bool InputConfig::load(json j)
{

	return true;
}

// Make sure nothing is malformed, or bad bindings are specified
string InputConfig::verify()
{
	for each (auto kv in bindings)
	{
		// Make sure function exists and is configurable
		if (InputState::functionsByName.count(kv.first) < 1 || InputState::configurableFunctions.count(kv.first) < 1)
		{
			// Return failure
			string failure = "Function '";
			failure += kv.first;
			failure += "' either does not exist or is not configurable.";
			return failure;
		}
		else
		{
			// Make sure both inputs are allowed in custom bindings
			if (InputState::bindableInputs.count(kv.second[0]) < 1 || InputState::bindableInputs.count(kv.second[1]) < 1)
			{
				// Return failure
				string failure = "Function '";
				failure += kv.first;
				failure += "' has a malformed input name, or the input is not allowed.";
				return failure;
			}
		}
	}
	return "";
}

json InputConfig::toJSON()
{
	json j;
	for each(auto kv in bindings)
	{
		// See if function is configurable
		if (InputState::configurableFunctions.count(kv.first) > 0)
		{
			// If so, add to JSON output
			j[kv.first].push_back(kv.second[0]);
			j[kv.first].push_back(kv.second[1]);
		}
	}
	return j;
}

void Input::setName(string n)
{
	name = n;
}
