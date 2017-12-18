#pragma once
#include "FileUtil.hpp"
#include "NesEmulator.hpp"
#include "N3sConsole.hpp"

#define MAX_STATES 16

class SaveState
{
public:
	SaveState(size_t size);
	~SaveState();
	size_t size;
	bool initialized = false;
	char* _data;
	// PNG data
};

// Collection of save states
namespace State {
	void init(string path, size_t size);		// Load all states from a folder
	void clear();								// Clear all states
	extern unique_ptr<SaveState> states[MAX_STATES];	// Stored states
	extern int selectedSlot;					// Currently selected slot
	void incrementSlot();						// Increment selected slot
	void decrementSlot();						// Decrement selected slot
	// screenshot set
	extern path stateDirectory;		// Where states are saved and loaded
	extern bool canSaveToDisk;		// Whether or not we can save to disk
	bool save();
	bool load();
	void * getDataPtr();		// Get pointer to current slot's data
	extern size_t stateSize;	// Buffer size for state, as returned by game when loaded
};