#include "stdafx.h"
#include "Gui.hpp"
#include "Overlay.hpp"
#include "Input.hpp"
#include "Editor.hpp"
#include "N3sMath.hpp"

shared_ptr<SpriteMesh> MeshInfo::m = nullptr;
shared_ptr<VoxelEditor> VoxelEditorInfo::e = nullptr;

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
		for (int i = 0; i < sceneCount; i++)
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
		if (highlightedTab == -1 && state < 1)
		{
			mouseCaptured = false;
		}
	}
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
	for (int i = 0; i < sceneCount; i++)
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
	for (int i = 0; i < sceneCount; i++)
	{
		string s = to_string(i);
		Overlay::drawString(2, currentY, 2, s);
		currentY += buttonHeight + buttonGap;
	}
}

bool PaletteSelector::update(bool mouseAvailable, shared_ptr<Scene> scene, shared_ptr<VoxelEditor> editor)
{
	N3sPalette * palette = scene->getSelectedPalette();
	int previouslySelectedIndex = selectedIndex;
	// See if we're modifying any selection's palette(s)
	int selected = scene->selection->selectedIndices.size();
	// Check for selections, but only if it's strictly OAM or NT, not both
	if (selected > 0)
	{
		bool allPalettesIdentical = true;
		// Check for all identical OAM
		if (selected > 0)
		{
			auto it = scene->selection->selectedIndices.begin();
			int p = scene->sprites[*it].palette;

			// See if this palette is different from one that may have been selected in the previous frame
			if (selectedPalette <= 0 && selectedPalette != p)
			{
				// Cancel out of color picker if open
				selectionLevel = 1;
			}
				
			// See if all selected items have the same palette
			for each(int i in scene->selection->selectedIndices)
			{
				if (scene->sprites[i].palette != p)
					allPalettesIdentical = false;
			}
			if (allPalettesIdentical)
				selectedPalette = p;
			else
				selectedPalette = -1;
		}
		// Check to see if any keys were pressed that would change selected color
		if (InputState::functions[voxeleditor_color0].activatedThisFrame)
		{
			selectColor(bg_swatch, scene, editor);
		}
		if (InputState::functions[voxeleditor_color1].activatedThisFrame)
		{
			selectColor(getRelativeColor(0), scene, editor);
		}
		if (InputState::functions[voxeleditor_color2].activatedThisFrame)
		{
			selectColor(getRelativeColor(1), scene, editor);
		}
		if (InputState::functions[voxeleditor_color3].activatedThisFrame)
		{
			selectColor(getRelativeColor(2), scene, editor);
		}
	}
	else
	{
		// No palette is selected
		selectedPalette = -1;
		//selectionLevel = 0;
	}
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
				}
			}
		}
		// Branch on bg color if we are in editor, in which case bottom half becomes eraser
		if (editor != nullptr)
		{
			if (mouseInRectangle(mouseX, mouseY, leftMargin + (boxSize * 12), 0, boxSize, boxSize))
			{
				anythingHighlighted = true;
				highlightedIndex = bg_swatch;
			}
			else if (mouseInRectangle(mouseX, mouseY, leftMargin + (boxSize * 12), boxSize, boxSize, boxSize))
			{
				anythingHighlighted = true;
				highlightedIndex = eraser;
			}
		}
		else
		{
			if (mouseInRectangle(mouseX, mouseY, leftMargin + (boxSize * 12), 0, boxSize, boxSize * 2))
			{
				anythingHighlighted = true;
				highlightedIndex = bg_swatch;
			}
		}
		// Check for palette switching and option highlights/presses
		if (mouseInRectangle(mouseX, mouseY, leftMargin + (boxSize * 13), 0, boxSize, boxSize))
		{
			anythingHighlighted = true;
			highlightedIndex = previous_palette;
		}
		if (mouseInRectangle(mouseX, mouseY, leftMargin + (boxSize * 14), 0, boxSize, boxSize))
		{
			anythingHighlighted = true;
			highlightedIndex = next_palettte;
		}
		// Check for highlights in color picker, if it is open
		if (selectedIndex >= 0 && selectionLevel == 3)
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
					}
				}
			}
			// Check for close button click
			if (mouseInRectangle(mouseX, mouseY, leftMargin, boxSize * 10, boxSize * 8, boxSize))
			{
				anythingHighlighted = true;
				highlightedIndex = colors_close;
			}
		}
		if (!anythingHighlighted)
			highlightedIndex = -1;
		else
		{
			if (state > 0)
			{
				mouseCaptured = true;
			}
			// React to button presses
			if (state == pressed)
			{
				if (highlightedIndex >= palette_start && highlightedIndex <= bg_swatch)
				{
					selectColor(highlightedIndex, scene, editor);
				}
				else if (highlightedIndex >= colors_start && highlightedIndex < colors_close)
				{
					// Color assignment
					int selectedColor = highlightedIndex - colors_start;
					if (selectedIndex < bg_swatch)
						palette->colorIndices[selectedIndex] = selectedColor;
					else
						palette->backgroundColorIndex = selectedColor;
				}
				else if (highlightedIndex == eraser)
				{
					selectionLevel = 2;
					selectedIndex = eraser;
					if (editor != nullptr)
					{
						selectColor(-1, scene, editor);
					}
				}
				else
				{
					selectionLevel = 0;
					// Some other button was pressed
					switch (highlightedIndex)
					{
					case colors_close:
						if (editor == nullptr)
							selectedIndex = -1;
						else
							selectionLevel = 2;
						break;
					case previous_palette:
						scene->selectPreviousPalette();
						break;
					case next_palettte:
						scene->selectNextPalette();
					case options_toggle:
						if (optionsOpen)
							optionsOpen = false;
						else
							optionsOpen = true;
						break;
					}
				}
			}
		}
	}
	// Return if mouse is still available
	if (mouseCaptured)
		return false;
	else
		return true;
}

void PaletteSelector::selectColor(int color, shared_ptr<Scene> scene, shared_ptr<VoxelEditor> editor)
{
	if (color >= 0)
	{
		int previouslySelectedIndex = selectedIndex;
		selectedIndex = color;
		int paletteHighlighted = floor(color / 3);
		int colorWithinPalette = color % 3;
		// See if this palette is different from the one that belongs to any selected sprites/nt
		if (selectedPalette != paletteHighlighted && color >= 0 && selectedIndex != bg_swatch)
		{
			scene->changeSelectionPalette(paletteHighlighted);
		}
		// Palette color/bg selection
		// no sprites selected
		if (selectedPalette == -1)
		{
			// open color picker
			selectionLevel = 3;
		}
		// sprites selected
		else if (selectedPalette == paletteHighlighted || selectedIndex == bg_swatch)
		{
			if (editor == nullptr)	// editor not open
			{
				selectionLevel = 3;
			}
			else					// voxel editor open
			{
				// See if color is already selected and open color picker if so
				if (previouslySelectedIndex == selectedIndex && selectionLevel <= 2)
					selectionLevel++;
				else
					selectionLevel = 2;
				// See if color is different, and close color picker if it was open
				if (selectionLevel == 3 && selectedIndex != previouslySelectedIndex)
					selectionLevel = 2;
				// Either way, change the selected color in the voxel editor
				if (selectedIndex < bg_swatch)
					editor->selectedColor = colorWithinPalette + 1;
				else
				{
					// TODO once I add background-colored voxels
					editor->selectedColor = 4;
				}
			}
		}
		else
		{
			if (editor != nullptr)
			{
				selectionLevel = 2;
				editor->selectedColor = colorWithinPalette + 1;
			}
			else
				selectionLevel = 1;
		}
	}
	else
	{
		editor->selectedColor = 0;
	}
}

int PaletteSelector::getRelativeColor(int i)
{
	return selectedPalette * 3 + i;
}

void PaletteSelector::render(shared_ptr<Scene> scene, shared_ptr<VoxelEditor> editor)
{
	N3sPalette * palette = scene->getSelectedPalette();
	Overlay::setColor(0, 0, 0, 128);
	Overlay::drawRectangle(leftMargin, 0, boxSize * 15, boxSize * 2);
	// Render palette selection
	if (selectedPalette >= 0)
	{
		int x = (selectedPalette % 4) * 3;
		int y = floor(selectedPalette / 4);
		Overlay::setColor(255, 255, 255, 64);
		Overlay::drawRectangle(leftMargin + (x * boxSize), y * boxSize, boxSize * 3, boxSize);
	}
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
				if (selectionLevel >= 2)
				{
					Overlay::drawRectangle(leftMargin + (i * boxSize), r * boxSize, boxSize, boxSize);
				}
			}
			if (highlightedIndex == index && selectedIndex != index)
			{
				if (selectionLevel >= 2)
				{
					Overlay::setColor(255, 255, 255, 64);
					Overlay::drawRectangle(leftMargin + (i * boxSize), r * boxSize, boxSize, boxSize);
				}
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
		if (editor != nullptr)
		{
			Overlay::setColor(255, 255, 255, 64);
			Overlay::drawRectangle(leftMargin + boxSize * 12, 0, boxSize, boxSize);
		}
		else
		{
			Overlay::setColor(255, 255, 255, 64);
			Overlay::drawRectangle(leftMargin + boxSize * 12, 0, boxSize, boxSize * 2);
		}
	}
	else if (selectedIndex == bg_swatch)
	{
		if (editor != nullptr)
		{
			Overlay::setColor(255, 255, 255, 128);
			Overlay::drawRectangle(leftMargin + boxSize * 12, 0, boxSize, boxSize);
		}
		else
		{
			Overlay::setColor(255, 255, 255, 128);
			Overlay::drawRectangle(leftMargin + boxSize * 12, 0, boxSize, boxSize * 2);
		}
	}
	// Render erasor, if selected and voxel editor open
	if (editor != nullptr)
	{
		if (highlightedIndex == eraser)
		{
			Overlay::setColor(255, 255, 255, 64);
			Overlay::drawRectangle(leftMargin + boxSize * 12, boxSize, boxSize, boxSize);
		}
		else if (selectedIndex == eraser)
		{
			Overlay::setColor(255, 255, 255, 128);
			Overlay::drawRectangle(leftMargin + boxSize * 12, boxSize, boxSize, boxSize);
		}
	}
	// Render menu buttons
	Overlay::setColor(255, 255, 255, 128);
	Overlay::drawString(leftMargin + (boxSize * 13) + 8, boxSize + 8, 2, std::to_string(scene->selectedPalette));
	Overlay::drawString(leftMargin + (boxSize * 13) + 8, 8, 2, "<");
	Overlay::drawString(leftMargin + (boxSize * 14) + 8, 8, 2, ">");
	Hue h = palette->getBackgroundColor();
	Overlay::setColor(h.red, h.green, h.blue, 1.0f);
	// Render bg color swatch at half-size if we're in voxel editor
	if (editor != nullptr)
	{
		Overlay::drawRectangle(leftMargin + boxSize * 12 + borderSize, borderSize, swatchSize, swatchSize);
	}
	else
	{
		Overlay::drawRectangle(leftMargin + boxSize * 12 + borderSize, borderSize, swatchSize, (swatchSize * 2) + (borderSize * 2));
	}
	// If a swatch is selected, render the pop-out color selector
	if (selectedIndex >= 0 && selectionLevel == 3)
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

bool MeshInfo::update(bool mouseAvailable, shared_ptr<SpriteMesh> mesh)
{
	m = mesh;
	int mouseCaptured = 0;
	// Check to see if mesh color has been clicked on
	int y = topMargin + (8 * 2 * scale);
	if (mouseAvailable)
	{
		MouseState state = InputState::keyboardMouse->mouseButtons[left_mouse].state;
		int mouseX = InputState::keyboardMouse->mouseX;
		int mouseY = InputState::keyboardMouse->mouseY;
		if (state == pressed && mouseInRectangle(mouseX, mouseY, 0, topMargin, 8 * width * scale, 8 * height * scale))
		{
			// See if we've pressed the "none" button
			if (mouseInRectangle(mouseX, mouseY, 0, y, 8 * scale, 8 * scale))
			{
				mesh->setOutline(-1);
			}
			else
			{
				// See if we're setting a differect color
				int x = 8 * scale;
				for (int i = 0; i <= 3; i++)
				{
					if (mouseInRectangle(mouseX, mouseY, x + (8 * scale * i), y, 8 * scale, 8 * scale))
					{
						mesh->setOutline(i);
					}
				}
			}
			return false;
		}
		else
		{
			return true;
		}
	}
	return true;
}

void MeshInfo::render()
{
	if (m != nullptr)
	{
		N3s3d::setShader(overlay);
		N3s3d::setGuiProjection();
		Overlay::setColor(0, 0, 0, 128);
		Overlay::drawRectangle(0, topMargin, width * 8 * scale, height * 8 * scale);
		Overlay::setColor(255, 255, 255, 128);
		string meshNumString = "MESH #" + std::to_string(m->id);
		Overlay::drawString(0, topMargin, scale, meshNumString);
		Overlay::drawString(0, topMargin + 8 * scale, scale, "OUTLINE:");
		// Draw selected outline
		Overlay::setColor(255, 255, 255, 64);
		Overlay::drawRectangle(8 * scale * (m->outlineColor + 1), topMargin + (8 * 2 * scale), 8 * scale, 8 * scale);
		Overlay::setColor(255, 255, 255, 128);
		Overlay::drawString(0, topMargin + 8 * 2 * scale, scale, "N0123");
	}
}

void MeshInfo::clear()
{
	m = nullptr;
}

bool VoxelEditorInfo::update(bool mouseAvailable, shared_ptr<VoxelEditor> editor)
{
	e = editor;
	if (mouseAvailable)
	{
		MouseState state = InputState::keyboardMouse->mouseButtons[left_mouse].state;
		int mouseX = InputState::keyboardMouse->mouseX;
		int mouseY = InputState::keyboardMouse->mouseY;
		if (state == pressed)
		{
			if (mouseInRectangle(mouseX, mouseY, 0, topMargin, 8 * width * scale, 8 * height * scale))
			{
				// Man I forget what this even is. can't wait to be done with the GUI
			}
			else if (mouseInRectangle(mouseX, mouseY, 0, topMargin + 6 * (8 * scale) + 4, 8 * scale * 5 + 4, 8 * scale + 4))
			{
				e = nullptr;
			}
		}
	}
	return false;
}

void VoxelEditorInfo::render()
{
	if (e != nullptr)
	{
		N3s3d::setShader(overlay);
		N3s3d::setGuiProjection();
		Overlay::setColor(0, 0, 0, 128);
		Overlay::drawRectangle(0, topMargin, width * 8 * scale, height * 8 * scale);
		Overlay::setColor(255, 255, 255, 128);
		int x, y, z;
		x = static_cast<int>(floor(e->workingX));
		y = static_cast<int>(floor(e->workingY));
		z = static_cast<int>(floor(e->workingZ));
		string positionString = to_string(x) + "," + to_string(y) + "," + to_string(z);
		Overlay::drawString(0, topMargin, scale, positionString);
		if (e->mouseInEditor)
		{
			x = e->mouseHighlight.x;
			y = e->mouseHighlight.y;
			z = e->mouseHighlight.z;
			positionString = to_string(x) + "," + to_string(y) + "," + to_string(z);
		}
		else
		{ 
			positionString = "-,-,-";
		}
		Overlay::drawString(0, topMargin + (8 * scale), scale, positionString);
		// Draw back button
		Overlay::setColor(0, 0, 0, 128);
		Overlay::drawRectangle(0, topMargin + 6 * (8 * scale) + 4, 8 * scale * 5 + 4, 8 * scale + 4);
		Overlay::setColor(255, 255, 255, 128);
		Overlay::drawString(2, topMargin + 5 * (8 * scale) + 6, scale, "CLOSE");
	}
}

void VoxelEditorInfo::clear()
{
	e = nullptr;
}