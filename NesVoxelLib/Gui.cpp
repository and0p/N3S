#include "stdafx.h"
#include "Gui.hpp"
#include "Overlay.hpp"
#include "Input.hpp"
#include "Editor.hpp"

bool SceneSelector::update(bool mouseAvailable)
{
	highlightedTab = -1;
	// Check for mouse hover, if mouse is available
	if (mouseAvailable || mouseCaptured)
	{
		MouseState state = InputState::keyboardMouse->mouseButtons[left_mouse].state;
		if (state < 1)
			mouseCaptured = 0;
		int mouseX = InputState::keyboardMouse->mouseX;
		int mouseY = InputState::keyboardMouse->mouseY;
		int currentY = buttonGap;
		// Check each button
		for (int i = 0; i < 8; i++)
		{
			// See if mouse is in button bounds
			if (mouseInRectangle(mouseX, mouseY, 0, currentY, buttonWidth, buttonHeight))
			{
				// Set this tab as being highlighted
				highlightedTab = i;
				if (state > 0)
				{
					mouseCaptured = true;
				}
				if (state == pressed)
				{
					Editor::setScene(i);
				}
			}
			currentY += buttonHeight + buttonGap;
		}
	}
	// Do other stuff

	// Return if mouse was captured
	return mouseCaptured;
}

void SceneSelector::render()
{
	Overlay::setColor(0, 0, 0, 60);
	int currentY = buttonGap;
	// Draw buttons
	for (int i = 0; i < 8; i++)
	{
		if (highlightedTab == i)
		{
			Overlay::setColor(150, 150, 150, 60);
			Overlay::drawRectangle(0, currentY, buttonWidth, buttonHeight);
			Overlay::setColor(0, 0, 0, 60);
		}
		else
		{
			Overlay::drawRectangle(0, currentY, buttonWidth, buttonHeight);
		}
		currentY += buttonHeight + buttonGap;
	}
	// Draw scene numbers
	currentY = buttonGap + 3;
	Overlay::setColor(255, 255, 255, 60);
	for (int i = 0; i < 8; i++)
	{
		string s(1, i + 48 + 1);
		Overlay::drawString(2, currentY, 2, s);
		currentY += buttonHeight + buttonGap;
	}
}
