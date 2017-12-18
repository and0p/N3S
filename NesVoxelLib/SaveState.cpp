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

	void init(string folder, size_t size)
	{
		stateSize = size;
		// See if we can save to disk
		stateDirectory = folder;
		if (exists(stateDirectory))
		{
			canSaveToDisk = true;
			// Load states that exists
			for (int i = 0; i < MAX_STATES; i++)
			{
				states[i] = make_unique<SaveState>(size);	// Create the state
				path currentPath = stateDirectory;
				currentPath.append(to_string(i) + fileExtension);
				if (exists(currentPath))
				{
					ifstream n3sFile(currentPath.c_str(), std::ifstream::in | ios::binary);	// Open read-only, binary mode
					int test = 0;
					if (n3sFile.good())
					{
						char c;
						char * dataMarker = states[i]->_data;
						for(int i = 0; i < stateSize; i++)
						{
							n3sFile.get(c);
							*dataMarker = c;
							dataMarker++;
							test++;
						}
						n3sFile.close();
						states[i]->initialized = true;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < MAX_STATES; i++)
				states[i] = make_unique<SaveState>(size);
		}
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
		N3sConsole::writeLine("SELECTED SLOT " + to_string(selectedSlot));
	}

	void decrementSlot()
	{
		selectedSlot--;
		if (selectedSlot < 0)
			selectedSlot = MAX_STATES - 1;
		N3sConsole::writeLine("SELECTED SLOT " + to_string(selectedSlot));
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
				stream.open(outputPath, std::ofstream::out | std::ofstream::binary);
				if (stream.good())
				{
					//std::string data(states[selectedSlot]->_data, stateSize);
					unsigned char * c = (unsigned char *)states[selectedSlot]->_data;
					for (int i = 0; i < stateSize; i++)
					{
						stream << *c;
						c++;
					}
				}
				stream.close();
			}
			N3sConsole::writeLine("SAVED TO SLOT " + to_string(selectedSlot));
			return true;
		}
		else
		{
			N3sConsole::writeLine("SAVE FAILED!");
			return false;
		}
	}

	bool load()
	{
		if (NesEmulator::loadState(states[selectedSlot]->_data, stateSize))
		{
			N3sConsole::writeLine("LOADED SLOT " + to_string(selectedSlot));
		}
		else
		{
			N3sConsole::writeLine("LOAD FAILED!");
			return false;
		}
	}

	void * getDataPtr()
	{
		return states[selectedSlot]->_data;
	}
}

SaveState::SaveState(size_t size) : size(size)
{
	_data = new char[size]();
	memset(_data, (unsigned int)0, size);
	initialized = false;
}

SaveState::~SaveState()
{
	delete[] _data;
}
