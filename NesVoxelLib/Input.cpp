#include "stdafx.h"
#include "Input.hpp"
#include "NesEmulator.hpp"
#include "libretro.h"

shared_ptr<KeyboardMouseDevice> InputState::keyboardMouse;
shared_ptr<GamepadDevice> InputState::gamepads[2];
InputFunction InputState::functions[inputFunctions::LAST];

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
	value = fabs(value);
	if (value >= deadzone)
	{
		setActive(true);
		if (framesActive > 0)
			setActivatedThisFrame(false);
		else
			setActivatedThisFrame(true);
		framesActive++;
		// Adjust value based on deadzone area
		value = value / (1.0f - deadzone);
	}
	else
	{
		setActivatedThisFrame(false);
		framesActive = 0;
		setActive(false);
	}
}

float AnalogInput::getValue()
{
	return value;
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
	for (int i = 0; i < totalKeys; i++)
	{
		keys[i]->update();
	}
}

GamepadDevice::GamepadDevice()
{
	for (int i = 0; i < xboxButtonCount; i++)
	{
		buttons[i] = make_shared<DigitalInput>();
	}
	analogInputs[leftXPos] = make_shared<AnalogInput>(false);
	analogInputs[leftXNeg] = make_shared<AnalogInput>(true);
	analogInputs[leftYPos] = make_shared<AnalogInput>(false);
	analogInputs[leftYNeg] = make_shared<AnalogInput>(true);
	analogInputs[rightXPos] = make_shared<AnalogInput>(false);
	analogInputs[rightXNeg] = make_shared<AnalogInput>(true);
	analogInputs[rightYPos] = make_shared<AnalogInput>(false);
	analogInputs[rightYNeg] = make_shared<AnalogInput>(true);
	analogInputs[lTrigger] = make_shared<AnalogInput>(false);
	analogInputs[rTrigger] = make_shared<AnalogInput>(false);
}

void GamepadDevice::update(bool connected, XINPUT_GAMEPAD gamepad)
{
	if (connected)
	{
		// Read buttons
		buttons[ba]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_A) != 0));
		buttons[bb]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_B) != 0));
		buttons[bx]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_X) != 0));
		buttons[by]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0));
		buttons[bstart]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_START) != 0));
		buttons[bselect]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0));
		buttons[bdLeft]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0));
		buttons[bdRight]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0));
		buttons[bdUp]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0));
		buttons[bdDown]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0));
		buttons[blb]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0));
		buttons[brb]->setActive(((gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0));
		// Read analog sticks
		analogInputs[leftX]->value = gamepad.sThumbLX / 32767.0f;
		analogInputs[leftY]->value = gamepad.sThumbLY / 32767.0f;
		analogInputs[rightX]->value = gamepad.sThumbRX / 32767.0f;
		analogInputs[rightY]->value = gamepad.sThumbRY / 32767.0f;
		analogInputs[lTrigger]->value = gamepad.bLeftTrigger / 255.0f;
		analogInputs[rTrigger]->value = gamepad.bRightTrigger / 255.0f;
	}
	else
	{
		for (int i = 0; i < xboxButtonCount; i++)
		{
			buttons[i]->setActive(false);
		}
		for (int i = 0; i < xboxAxisCount; i++)
		{
			analogInputs[i]->value = 0.0f;
		}
	}
}

InputState::InputState()
{
	keyboardMouse = make_shared<KeyboardMouseDevice>();
	gamepads[0] = make_shared<GamepadDevice>();
	gamepads[1] = make_shared<GamepadDevice>();
	createBindings();
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
		gamepads[0]->update(true, state.Gamepad);
	}
	else
	{
		gamepads[0]->update(false, state.Gamepad);
	}
}

void InputState::refreshInput()
{
	checkGamePads();
	keyboardMouse->update();
	// Update bindings and send input to NES
	for (int i = 0; i < inputFunctions::LAST; i++)
	{
		functions[i].update();
	}
	sendNesInput();
}

void InputState::sendNesInput()
{
	if (functions[nes_p1_a].active)
	{
		int test = 0;
	}
	// Player 1
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_A] = functions[nes_p1_a].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_B] = functions[nes_p1_b].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_SELECT] = functions[nes_p1_select].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_START] = functions[nes_p1_start].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_UP] = functions[nes_p1_up].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_DOWN] = functions[nes_p1_down].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_LEFT] = functions[nes_p1_left].active;
	NesEmulator::inputs[0][RETRO_DEVICE_ID_JOYPAD_RIGHT] = functions[nes_p1_right].active;
	// Player 2
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_A] = functions[nes_p2_a].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_B] = functions[nes_p2_b].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_SELECT] = functions[nes_p2_select].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_START] = functions[nes_p2_start].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_UP] = functions[nes_p2_up].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_DOWN] = functions[nes_p2_down].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_LEFT] = functions[nes_p2_left].active;
	NesEmulator::inputs[1][RETRO_DEVICE_ID_JOYPAD_RIGHT] = functions[nes_p2_right].active;
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
		if (b.input->activatedThisFrame)
			activatedThisFrame = true;
		if (b.input->framesActive > framesActive)
			framesActive = b.input->framesActive;
		if (b.input->getValue() > value)
			value = b.input->getValue();
	}
}

void InputState::createBindings()
{
	// Crappy hard-coded test bindings
	// TODO: load from a proper config, build menu to edit

	// NES Player 1 controls
	functions[nes_p1_a].bindings.push_back({ InputState::keyboardMouse->keys[0x4B] });
	functions[nes_p1_b].bindings.push_back({ InputState::keyboardMouse->keys[0x4A] });
	functions[nes_p1_select].bindings.push_back({ InputState::keyboardMouse->keys[0x55] });
	functions[nes_p1_start].bindings.push_back({ InputState::keyboardMouse->keys[0x49] });
	functions[nes_p1_up].bindings.push_back({ InputState::keyboardMouse->keys[0x57] });
	functions[nes_p1_down].bindings.push_back({ InputState::keyboardMouse->keys[0x53] });
	functions[nes_p1_left].bindings.push_back({ InputState::keyboardMouse->keys[0x41] });
	functions[nes_p1_right].bindings.push_back({ InputState::keyboardMouse->keys[0x44] });
	// Camera controls
	functions[cam_up].bindings.push_back({ keyboardMouse->keys[VK_UP] });
	functions[cam_down].bindings.push_back({ keyboardMouse->keys[VK_DOWN] });
	functions[cam_left].bindings.push_back({ keyboardMouse->keys[VK_LEFT] });
	functions[cam_right].bindings.push_back({ keyboardMouse->keys[VK_RIGHT] });
	functions[cam_pan_in].bindings.push_back({ keyboardMouse->keys[VK_OEM_PERIOD] });
	functions[cam_pan_out].bindings.push_back({ keyboardMouse->keys[VK_OEM_COMMA] });
}