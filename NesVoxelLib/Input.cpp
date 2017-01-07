#include "stdafx.h"
#include "Input.hpp"
#include "NesEmulator.hpp"
#include "libretro.h"

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
	if (value > deadzone)
	{
		if (framesActive > 0)
			setActivatedThisFrame(false);
		else
			setActivatedThisFrame(true);
		setFramesActive(framesActive + 1);
		setActive(true);
	}
	else
	{
		setActive(false);
		setActivatedThisFrame(false);
		setFramesActive(0);
	}
}

float AnalogInput::getValue()
{
	return 0.0f;
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
	keys[key]->setActive(true);
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

void GamepadDevice::update(XINPUT_GAMEPAD gamepad)
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
