#include "stdafx.h"
#include "Gui.hpp"
#include "Overlay.hpp"
#include "Input.hpp"
#include "Editor.hpp"

bool SceneSelector::update(bool mouseAvailable)
{
	// Check for mouse hover, if mouse is available
	if (mouseAvailable || mouseCaptured)
	{
		bool anythingHighlighted = false;
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
				anythingHighlighted = true;
				highlightedTab = i;
				if (state > 0)
				{
					mouseCaptured = true;
				}
				if (state == pressed)
				{
					Editor::setScene(i);
					mouseCaptured = false;
				}
			}
			currentY += buttonHeight + buttonGap;
		}
		if (!anythingHighlighted)
			highlightedTab = -1;
		if (highlightedTab == -1 && state )
		{
			mouseCaptured = false;
		}
	}
	// Do other stuff

	// Return if mouse is still available
	if (mouseCaptured)
		return false;
	else
		return true;
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
			Overlay::setColor(150, 150, 150, 64);
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
	Overlay::setColor(255, 255, 255, 128);
	for (int i = 0; i < 8; i++)
	{
		string s(1, i + 48 + 1);
		Overlay::drawString(2, currentY, 2, s);
		currentY += buttonHeight + buttonGap;
	}
}

bool PaletteSelector::update(bool mouseAvailable, shared_ptr<Scene> scene)
{
	N3sPalette * palette = scene->getSelectedPalette();
	// Check for mouse hover, if mouse is available
	if (mouseAvailable || mouseCaptured)
	{
		MouseState state = InputState::keyboardMouse->mouseButtons[left_mouse].state;
		if (state < 1)
			mouseCaptured = 0;
		int mouseX = InputState::keyboardMouse->mouseX;
		int mouseY = InputState::keyboardMouse->mouseY;
		int currentX = leftMargin;
		int currentY = 0;
		bool anythingHighlighted = false;	// Track if anything was highlighted
		// Check for highlights on palette colors + background color
		for (int r = 0; r < 2; r++)
		{
			for (int i = 0; i < 12; i++)
			{
				if (mouseInRectangle(mouseX, mouseY, leftMargin + (i * boxSize), r * boxSize, boxSize, boxSize))
				{
					// Set this tab as being highlighted
					anythingHighlighted = true;
					highlightedIndex = (r * 12) + i;
					if (state > 0)
					{
						mouseCaptured = true;
					}
					if (state == pressed)
					{
						selectedIndex = highlightedIndex;
						mouseCaptured = false;
					}
				}
			}
		}
		if (mouseInRectangle(mouseX, mouseY, leftMargin + (boxSize * 12), 0, boxSize, boxSize * 2))
		{
			anythingHighlighted = true;
			highlightedIndex = bg_swatch;
			if (state > 0)
			{
				mouseCaptured = true;
			}
			if (state == pressed)
			{
				selectedIndex = bg_swatch;
				mouseCaptured = false;
			}
		}
		// Check for highlights in color picker, if it is open
		if (selectedIndex >= 0)
		{
			// Check for color highlight / selection
			for (int x = 0; x < 8; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					if (mouseInRectangle(mouseX, mouseY, leftMargin + (x * boxSize), (boxSize * 2) + (y * boxSize), boxSize, boxSize))
					{
						anythingHighlighted = true;
						highlightedIndex = colors_start + (y * 8) + x;
						if (state > 0)
						{
							mouseCaptured = true;
						}
						if (state == pressed)
						{
							int selectedColor = (y * 8) + x;
							if (selectedIndex < bg_swatch)
								palette->colorIndices[selectedIndex] = selectedColor;
							else
								palette->backgroundColorIndex = selectedColor;
							mouseCaptured = false;
						}
					}
				}
			}
			// Check for close button click
			if (mouseInRectangle(mouseX, mouseY, leftMargin, boxSize * 10, boxSize * 8, boxSize))
			{
				anythingHighlighted = true;
				highlightedIndex = colors_close;
				if (state > 0)
				{
					mouseCaptured = true;
				}
				if (state == pressed)
				{
					// Close the selection windows
					selectedIndex = -1;
					anythingHighlighted = false;
					mouseCaptured = false;
				}
			}
		}
		if (!anythingHighlighted)
			highlightedIndex = -1;
	}
	// Return if mouse is still available
	if (mouseCaptured)
		return false;
	else
		return true;
}

void PaletteSelector::render(shared_ptr<Scene> scene)
{
	N3sPalette * palette = scene->getSelectedPalette();
	Overlay::setColor(0, 0, 0, 128);
	Overlay::drawRectangle(leftMargin, 0, boxSize * 15, boxSize * 2);
	// Render color swatches
	for (int r = 0; r < 2; r++)
	{
		for (int i = 0; i < 12; i++)
		{
			int index = (r * 12) + i;
			// See if this is highlighted or selected and draw a highlight box if so
			if (selectedIndex == index)
			{
				Overlay::setColor(255, 255, 255, 128);
				Overlay::drawRectangle(leftMargin + (i * boxSize), r * boxSize, boxSize, boxSize);
			}
			if (highlightedIndex == index && selectedIndex != index)
			{
				Overlay::setColor(255, 255, 255, 64);
				Overlay::drawRectangle(leftMargin + (i * boxSize), r * boxSize, boxSize, boxSize);
			}
			// Set the overlay color to the color of this palette swatch
			int color = palette->colorIndices[(r * 12) + i];
			Hue h = N3sPalette::getHue(color);
			Overlay::setColor(h.red, h.green, h.blue, 1.0f);
			// Render the swatch
			Overlay::drawRectangle(leftMargin + (i * boxSize) + borderSize, (r * boxSize) + borderSize, swatchSize, swatchSize);
		}
	}
	// Render the BG color swatch
	if (highlightedIndex == bg_swatch)
	{
		Overlay::setColor(255, 255, 255, 64);
		Overlay::drawRectangle(leftMargin + boxSize * 12, 0, boxSize, boxSize * 2);
	}
	else if (selectedIndex == bg_swatch)
	{
		Overlay::setColor(255, 255, 255, 128);
		Overlay::drawRectangle(leftMargin + boxSize * 12, 0, boxSize, boxSize * 2);
	}
	// Render menu buttons
	Overlay::setColor(255, 255, 255, 128);
	Overlay::drawString(leftMargin + (boxSize * 13) + 8, boxSize + 8, 2, std::to_string(scene->selectedPalette));
	Overlay::drawString(leftMargin + (boxSize * 13) + 8, 8, 2, "<");
	Overlay::drawString(leftMargin + (boxSize * 14) + 8, 8, 2, ">");
	Hue h = palette->getBackgroundColor();
	Overlay::setColor(h.red, h.green, h.blue, 1.0f);
	Overlay::drawRectangle(leftMargin + boxSize * 12 + borderSize, borderSize, swatchSize, (swatchSize * 2) + (borderSize * 2));
	// If a swatch is selected, render the pop-out color selector
	if (selectedIndex >= 0)
	{
		Overlay::setColor(0, 0, 0, 128);
		int top = boxSize * 2;
		Overlay::drawRectangle(leftMargin, top, boxSize * 8, boxSize * 9);
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				int index = (y * 8) + x;
				// See if this is the currently selected color and draw border if so
				if (index == palette->colorIndices[selectedIndex])
				{
					Overlay::setColor(255, 255, 255, 128);
					Overlay::drawRectangle(leftMargin + (x * boxSize), top + (y * boxSize), boxSize, boxSize);
				}
				// Show highlight if highlighted but not selected
				if (index + colors_start == highlightedIndex && index != palette->colorIndices[selectedIndex])
				{
					Overlay::setColor(255, 255, 255, 64);
					Overlay::drawRectangle(leftMargin + (x * boxSize), top + (y * boxSize), boxSize, boxSize);
				}
				Hue h = N3sPalette::getHue(index);
				Overlay::setColor(h.red, h.green, h.blue, 1.0f);
				Overlay::drawRectangle(leftMargin + (x * boxSize) + borderSize, top + (y * boxSize) + borderSize, swatchSize, swatchSize);
			}
		}
		// Draw close button assets
		if (highlightedIndex == colors_close)
		{
			Overlay::setColor(255, 255, 255, 64);
			Overlay::drawRectangle(leftMargin, boxSize * 10, boxSize * 8, boxSize);
		}
		Overlay::setColor(255, 255, 255, 128);
		Overlay::drawString(leftMargin + (boxSize * 4) - 6, boxSize * 10 + 6, 2, "X");
	}
}
