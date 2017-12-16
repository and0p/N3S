#include "stdafx.h"
#include "SaveState.hpp"
#include <iostream>
#include <fstream>

const string fileExtension = ".sav";

namespace State {

	size_t stateSize = 0;
	int selectedSlot = 0;
	unique_ptr<SaveState> states[MAX_STATES];
	path stateDirectory = "";
	bool canSaveToDisk = false;

	void init(string path, size_t size)
	{
		stateSize = size;
		// See if we can save to disk
		stateDirectory = path;
		if (exists(stateDirectory))
			canSaveToDisk = true;
		for (int i = 0; i < MAX_STATES; i++)
			states[i] = make_unique<SaveState>(size);
	}

	void clear()
	{
		stateDirectory.clear();
		canSaveToDisk = false;
		for (int i = 0; i < MAX_STATES; i++)
			states[i].reset();
	}

	void incrementSlot()
	{
		selectedSlot++;
		if (selectedSlot >= MAX_STATES)
			selectedSlot = 0;
	}

	void decrementSlot()
	{
		selectedSlot--;
		if (selectedSlot < 0)
			selectedSlot = MAX_STATES - 1;
	}

	bool save()
	{
		bool success = NesEmulator::saveState(states[selectedSlot]->_data, stateSize);
		if (success)
		{
			states[selectedSlot]->initialized = true;
			// Save to disk, if possible
			if (canSaveToDisk)
			{
				ofstream stream;
				path outputPath = stateDirectory;
				outputPath.append(to_string(selectedSlot) + fileExtension);
				stream.open(outputPath);
				if (stream.good())
				{
					std::string data(states[selectedSlot]->_data, stateSize);
					stream << data;
				}
				stream.close();
			}
			N3sConsole::writeLine("Saved to slot " + to_string(selectedSlot));
			return true;
		}
		else
		{
			N3sConsole::writeLine("Save failed!");
			return false;
		}
	}

	bool load()
	{
		return NesEmulator::loadState(states[selectedSlot]->_data, stateSize);
	}

	void * getDataPtr()
	{
		return states[selectedSlot]->_data;
	}
}

SaveState::SaveState(size_t size) : size(size)
{
	_data = new char[size]();
	memset(_data, 0, size);
	initialized = true;
}

SaveState::~SaveState()
{
	delete[] _data;
}
