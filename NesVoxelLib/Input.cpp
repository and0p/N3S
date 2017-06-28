#include "stdafx.h"
#include "Input.hpp"
#include "NesEmulator.hpp"
#include "libretro.h"

shared_ptr<KeyboardMouseDevice> InputState::keyboardMouse;
shared_ptr<GamepadDevice> InputState::gamepads[2];
InputFunction InputState::functions[INPUTCOUNT];

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

GamepadDevice::GamepadDevice()
{
	for (int i = 0; i < BUTTONCOUNT; i++)
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
		analogInputs[leftXNeg]->value = gamepad.sThumbLX / 32767.0f;
		analogInputs[leftXPos]->value = gamepad.sThumbLX / 32767.0f;
		analogInputs[leftYNeg]->value = gamepad.sThumbLY / 32767.0f;
		analogInputs[leftYPos]->value = gamepad.sThumbLY / 32767.0f;
		analogInputs[rightXNeg]->value = gamepad.sThumbRX / 32767.0f;
		analogInputs[rightXPos]->value = gamepad.sThumbRX / 32767.0f;
		analogInputs[rightYNeg]->value = gamepad.sThumbRY / 32767.0f;
		analogInputs[rightYPos]->value = gamepad.sThumbRY / 32767.0f; 
		analogInputs[lTrigger]->value = gamepad.bLeftTrigger / 255.0f;
		analogInputs[rTrigger]->value = gamepad.bRightTrigger / 255.0f;

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
	for (int i = 0; i < INPUTCOUNT; i++)
	{
		if (i == 18)
		{
			int boop = 0;
		}
		functions[i].update();
	}
	sendNesInput();
	// DEBUG: Check VK_MENU
	if (keyboardMouse->keys[VK_MENU]->active)
		int test = 0;
	if (functions[selection_remove].active)
		int testtt = 0;
}

void InputState::sendNesInput()
{
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

	functions[nes_p1_up].bindings.push_back({ gamepads[0]->analogInputs[leftYPos] });
	functions[nes_p1_down].bindings.push_back({ gamepads[0]->analogInputs[leftYNeg] });
	functions[nes_p1_left].bindings.push_back({ gamepads[0]->analogInputs[leftXNeg] });
	functions[nes_p1_right].bindings.push_back({ gamepads[0]->analogInputs[leftXPos] });

	// Camera controls
	functions[cam_up].bindings.push_back({ keyboardMouse->keys[VK_UP] });
	functions[cam_down].bindings.push_back({ keyboardMouse->keys[VK_DOWN] });
	functions[cam_left].bindings.push_back({ keyboardMouse->keys[VK_LEFT] });
	functions[cam_right].bindings.push_back({ keyboardMouse->keys[VK_RIGHT] });
	functions[cam_pan_in].bindings.push_back({ keyboardMouse->keys[VK_OEM_PERIOD] });
	functions[cam_pan_out].bindings.push_back({ keyboardMouse->keys[VK_OEM_COMMA] });

	functions[cam_left].bindings.push_back({ gamepads[0]->analogInputs[rightXNeg] });
	functions[cam_right].bindings.push_back({ gamepads[0]->analogInputs[rightXPos] });
	functions[cam_up].bindings.push_back({ gamepads[0]->analogInputs[rightYPos] });
	functions[cam_down].bindings.push_back({ gamepads[0]->analogInputs[rightYNeg] });

	// Switching between editor and game
	functions[tog_game].bindings.push_back({ keyboardMouse->keys[VK_OEM_1] });		// semicolon
	functions[tog_editor].bindings.push_back({ keyboardMouse->keys[VK_OEM_7] });	// single quote

	// Editor controls
	functions[selection_add].bindings.push_back({ keyboardMouse->keys[VK_SHIFT] });
	functions[selection_remove].bindings.push_back({ keyboardMouse->keys[VK_MENU] });
	functions[selection_copy].bindings.push_back({ keyboardMouse->keys[VK_CONTROL] });
	functions[selection_delete].bindings.push_back({ keyboardMouse->keys[VK_DELETE] });
	functions[selection_deselect].bindings.push_back({ keyboardMouse->keys[VK_ESCAPE] });
	functions[editor_alt].bindings.push_back({ keyboardMouse->keys[VK_MENU] });
	functions[editor_moveleft].bindings.push_back({ keyboardMouse->keys[VK_LEFT] });
	functions[editor_moveright].bindings.push_back({ keyboardMouse->keys[VK_RIGHT] });
	functions[editor_moveup].bindings.push_back({ keyboardMouse->keys[VK_UP] });
	functions[editor_movedown].bindings.push_back({ keyboardMouse->keys[VK_DOWN] });
	functions[voxeleditor_setvoxel].bindings.push_back({ keyboardMouse->keys[0x4B] });
	functions[voxeleditor_deletevoxel].bindings.push_back({ keyboardMouse->keys[0x4A] });
	functions[voxeleditor_movein].bindings.push_back({ keyboardMouse->keys[0x55] });
	functions[voxeleditor_moveout].bindings.push_back({ keyboardMouse->keys[0x49] });
	functions[voxeleditor_color0].bindings.push_back({ keyboardMouse->keys[0x30] });
	functions[voxeleditor_color0].bindings.push_back({ keyboardMouse->keys[VK_NUMPAD0] });
	functions[voxeleditor_color1].bindings.push_back({ keyboardMouse->keys[0x31] });
	functions[voxeleditor_color1].bindings.push_back({ keyboardMouse->keys[VK_NUMPAD1] });
	functions[voxeleditor_color2].bindings.push_back({ keyboardMouse->keys[0x32] });
	functions[voxeleditor_color2].bindings.push_back({ keyboardMouse->keys[VK_NUMPAD2] });
	functions[voxeleditor_color3].bindings.push_back({ keyboardMouse->keys[0x33] });
	functions[voxeleditor_color3].bindings.push_back({ keyboardMouse->keys[VK_NUMPAD3] });
	functions[voxeleditor_exit].bindings.push_back({ keyboardMouse->keys[VK_ESCAPE] });
	// Editor copy/paste
	Binding copyBinding = { keyboardMouse->keys[0x43] };	// C
	copyBinding.ctrl = true;
	Binding pasteBinding = { keyboardMouse->keys[0x56] };	// V
	pasteBinding.ctrl = true;
	functions[editor_copy].bindings.push_back({ copyBinding });
	functions[editor_paste].bindings.push_back({ pasteBinding });
	functions[palette_copy].bindings.push_back({ keyboardMouse->keys[0x4F] });
	functions[palette_paste].bindings.push_back({ keyboardMouse->keys[0x50] });
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
