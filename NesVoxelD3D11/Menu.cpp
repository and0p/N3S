#pragma once
#include "pch.h"
#include "Menu.h"
#include <unordered_map>

#define NES_SUBMENU_NO 1
#define CONFIG_SUBMENU_NO 2
#define REGISTER_OFFSET 3	// Gotta include the divider!!!

#define OVERRIDE_OPTION_COUNT 6

#define MUTE_AUDIO_INDEX 0

struct RegisterBinding {
	RegisterOption option;
	UINT groupID;
	UINT menuID[OVERRIDE_OPTION_COUNT];
};

struct ButtonBinding {
	RegisterOption option;
	int value;
};

RegisterBinding registerMenuItems[REGISTER_OPTION_SIZE] = {
	mirror_selection, 0, { ID_MIRROR_NOO, ID_MIRROR_VERTICAL, ID_MIRROR_HORIZONTAL, ID_MIRROR_SINGLE, ID_MIRROR_FULL, ID_MIRROR_DIAGONAL },
	oam_pattern, 1 ,{ ID_OAMTABLE_NOO, ID_OAMTABLE_LEFT, ID_OAMTABLE_RIGHT, 0, 0, 0 },
	nt_pattern, 2, { ID_NTTABLE_NOO, ID_NTTABLE_LEFT, ID_NTTABLE_RIGHT, 0, 0, 0 },
	sprite8x16, 3, { ID_SPRITE8X16_NOO, ID_SPRITE8X16_DISABLED, ID_SPRITE8X16_ENABLED, 0, 0, 0 },
	render_oam, 4, { ID_RENDEROAM_NOO, ID_RENDEROAM_DISABLED, ID_RENDEROAM_ENABLED, 0, 0, 0 },
	render_nt, 5, { ID_RENDERNT_NOO, ID_RENDERNT_DISABLED, ID_RENDERNT_ENABLED, 0, 0, 0 },
	render_oam_left, 6, { ID_RENDERLEFTOAM_NOO, ID_RENDERLEFTOAM_DISABLED, ID_RENDERLEFTOAM_ENABLED, 0, 0, 0 },
	render_nt_left, 7, { ID_RENDERLEFTNT_NOO, ID_RENDERLEFTNT_DISABLED, ID_RENDERLEFTNT_ENABLED, 0, 0, 0 },
	greyscale, 8, { ID_GREYSCALE_NOO, ID_GREYSCALE_DISABLED, ID_GREYSCALE_ENABLED, 0, 0, 0 },
	emphasize_red, 9, { ID_EMPHASIZERED_NOO, ID_EMPHASIZERED_DISABLED, ID_EMPHASIZERED_ENABLED, 0, 0, 0 },
	emphasize_green, 10, { ID_EMPHASIZEGREEN_NOO, ID_EMPHASIZEGREEN_DISABLED, ID_EMPHASIZEGREEN_ENABLED, 0, 0, 0 },
	emphasize_blue, 11, { ID_EMPHASIZEBLUE_NOO, ID_EMPHASIZEBLUE_DISABLED, ID_EMPHASIZEBLUE_ENABLED, 0, 0, 0 }
};

unordered_map<UINT, ButtonBinding> buttonBindings;

// Map all buttons to their registers and values
void initMenu() {
	for (int r = 0; r < REGISTER_OPTION_SIZE; r++)
	{
		for (int i = 0; i < OVERRIDE_OPTION_COUNT; i++)
		{
			RegisterBinding binding = registerMenuItems[r];
			// See if this option has a button
			if (binding.menuID[i] > 0)
			{
				// Add to the map of buttons, with the register option -1 (so that "no override" is -1, rather than 0)
				buttonBindings[binding.menuID[i]] = { binding.option, i - 1 };
			}
		}
	}
}

void updateMenu(HMENU hmenu, N3sApp * app)
{
	// Get the submenus
	HMENU nesSubmenu = GetSubMenu(hmenu, NES_SUBMENU_NO);
	HMENU configSubmenu = GetSubMenu(hmenu, CONFIG_SUBMENU_NO);
	// Enable and disable relevant controls if game has not yet loaded
	if (app->loaded)
	{
		EnableMenuItem(nesSubmenu, ID_NES_POWER, MF_ENABLED);
		EnableMenuItem(nesSubmenu, ID_NES_RESET, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(nesSubmenu, ID_NES_POWER, MF_DISABLED);
		EnableMenuItem(nesSubmenu, ID_NES_RESET, MF_DISABLED);
	}
	// Update pause status
	CheckMenuItem(nesSubmenu, ID_NES_PAUSE, (app->emulationPaused ? MF_CHECKED : MF_UNCHECKED));
	// Update register display
	for (UINT i = 0; i < REGISTER_OPTION_SIZE; i++)
	{
		RegisterOption o = (RegisterOption)i;
		// See if register active, overridden or not, and check the menu item if so
		if (N3sConfig::isRegisterActive(o))
		{
			CheckMenuItem(nesSubmenu, REGISTER_OFFSET + registerMenuItems[i].groupID, MF_CHECKED | MF_BYPOSITION);
		}
		else
		{
			CheckMenuItem(nesSubmenu, REGISTER_OFFSET + registerMenuItems[i].groupID, MF_UNCHECKED | MF_BYPOSITION);
		}
		// Check the correct submenu item, indicating override status
		HMENU registerSubmenu = GetSubMenu(nesSubmenu, REGISTER_OFFSET + registerMenuItems[i].groupID);
		for (int r = 0; r < OVERRIDE_OPTION_COUNT; r++)
			CheckMenuItem(registerSubmenu, r, ((N3sConfig::registers[o] == r - 1) ? MF_CHECKED : MF_UNCHECKED) | MF_BYPOSITION);
		// Set group label bold (or asterisk?) to indicate override status
	}
	// See if anything is overridden and enable override reset button
	EnableMenuItem(nesSubmenu, ID_NES_RESETREGISTEROVERRIDES, (N3sConfig::anyRegistersOveridden() ? MF_ENABLED : MF_DISABLED));
	// Update mute button
	CheckMenuItem(configSubmenu, MUTE_AUDIO_INDEX, (N3sConfig::options[mute_audio] ? MF_CHECKED : MF_UNCHECKED) | MF_BYPOSITION);
}

void updateNesRegistry(UINT button)
{
	// See if this button actually binds to anything
	if (buttonBindings.count(button) > 0)
	{
		N3sConfig::setRegisterOverride(buttonBindings[button].option, buttonBindings[button].value);
	}
}
