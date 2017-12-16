#include "stdafx.h"
#include "N3sConfig.hpp"
#include "N3sApp.hpp"
#include "N3sConsole.hpp"

#define CONFIG_NAME		"\\config.json";

bool N3sConfig::options[N3S_OPTION_SIZE];
int N3sConfig::registers[REGISTER_OPTION_SIZE];
UINT N3sConfig::nesRegisters[REGISTER_OPTION_SIZE];

void N3sConfig::init()
{
	// Set default options
	options[mute_audio] = false;
	// Set all registers to not override NES
	resetRegisterOverrides();
}

bool N3sConfig::anyRegistersOveridden()
{
	for (int i = 0; i < REGISTER_OPTION_SIZE; i++)
		if (registers[i] != NO_OVERRIDE)
			return true;
	return false;
}

void N3sConfig::load()
{
	// TODO: try/catch, etc

	// Grab config file
	string configPath = N3sApp::applicationDirectory + CONFIG_NAME;
	ifstream configFile(configPath.c_str());
	if (configFile.good())
	{
		json j = json::parse(configFile);
		if (j.count("input") > 0)
		{
			InputConfig ic;
			bool success = ic.load(j["input"]);
			if (success)
			{
				string applyResult = InputState::applyInputConfig(ic);
				if (applyResult != "")
				{
					// Load default
					N3sConsole::writeLine("Failed to load input config:" + applyResult);
				}
			}
			else
			{
				// Load default
			}
		}
	}
	else
	{
		// Default configs
		save();
	}
}

void N3sConfig::save()
{
	// TODO: try/catch, etc

	// Create JSON from various config parts
	json j;
	j["input"] = InputState::getInputConfig().toJSON();
	string output = j.dump(4);
	// Save to file with path specified
	string configPath = N3sApp::applicationDirectory + CONFIG_NAME;
	ofstream myfile;
	myfile.open(configPath.c_str());
	myfile << output;
	myfile.close();
}

void N3sConfig::update(shared_ptr<PpuSnapshot> snapshot)
{
	// Snag NES registers, if the game is running, otherwise assume off
	if (snapshot != nullptr)
	{
		for (int i = 0; i < REGISTER_OPTION_SIZE; i++)
		{
			nesRegisters[i] = snapshot->registerOptions[i];
		}
	}
	else
	{
		for (int i = 0; i < REGISTER_OPTION_SIZE; i++)
		{
			nesRegisters[i] = 0;
		}
	}
}

bool N3sConfig::getOption(N3sOption o)
{
	return options[o];
}

void N3sConfig::setOption(N3sOption o, bool val)
{
	options[0] = val;
}

void N3sConfig::toggleOption(N3sOption o)
{
	options[0] = (options[0]) ? false : true;
}

UINT N3sConfig::getRegisterOverride(RegisterOption o)
{
	return registers[o];
}

bool N3sConfig::isRegisterActive(RegisterOption o)
{
	// See if we're overriding, otherwise see if NES has it active
	if (registers[o] >= OVERRIDE_TRUE || (nesRegisters[o] > 0 && registers[o] == NO_OVERRIDE))
		return true;
	else
		return false;
}

void N3sConfig::setRegisterOverride(RegisterOption o, int val)
{
	registers[o] = val;
}

void N3sConfig::resetRegisterOverrides()
{
	for (int i = 0; i < REGISTER_OPTION_SIZE; i++)
	{
		registers[i] = NO_OVERRIDE;
	}
}

bool N3sConfig::anyRegisterOverridden()
{
	for (int i = 0; i < REGISTER_OPTION_SIZE; i++)
		if (registers[i] > NO_OVERRIDE)
			return true;
	return false;
}