#include "stdafx.h"
#include "N3sConfig.hpp"
#include "N3sApp.hpp"
#include "N3sConsole.hpp"

#define CONFIG_NAME "\\config.json";

void N3sConfig::load()
{
	// TODO: try/catch, etc

	// Grab config file
	string configPath = N3sApp::applicationDirectory + CONFIG_NAME;
	ifstream configFile(configPath.c_str());
	if (configFile.good())
	{
		json j(configFile);
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