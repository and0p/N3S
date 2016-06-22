#pragma once

#include <vector>
#include <map>

struct PatternTable {
	char data[4096];
};

struct NameTable {
	char data[1024];
};

struct ScrollState {
	int x = 0;
	int y = 0;
	int highOrderBit = 0;
};

enum ScrollDataType { x, y, high };

struct VxlRawPPU {
public:
	char ctrl;
	char mask;
	char status;
	char palette[32];
	char oam[256];
	PatternTable patternTables[2];
	NameTable nameTables[4];
	std::map<int, ScrollState> scrollStates;
	int mostRecentScanLineModified = 0;
	void writeScrollValue(int scanline, ScrollDataType type, int data);
	void clearScrollValue(int scanline);
	void reset();
private:
};


inline void VxlRawPPU::writeScrollValue(int scanline, ScrollDataType type, int data) {
	// Increment the scanline, assuming these changes are applying to next line
	scanline++;
	// Set scanline to 0 if we are outside of visible range, as it will all wrap back to rendering at 0
	if (scanline < 0 || scanline > 240)
		scanline = 0;
	// See if we have a ScrollState for this scanline, if not create one
	if (scrollStates.count(scanline) == 0)
	{
		scrollStates[scanline] = ScrollState();
		// Grab the most recent high-order bit, in case the game doesn't set it at every scroll
		scrollStates[scanline].highOrderBit = scrollStates[mostRecentScanLineModified].highOrderBit;
	}
	mostRecentScanLineModified = scanline;
	switch (type)
	{
	case x:
		scrollStates[scanline].x = data;
		break;
	case y:
		scrollStates[scanline].y = data;
		break;
	case high:
		scrollStates[scanline].highOrderBit = data;
		break;
	}
}

inline void VxlRawPPU::clearScrollValue(int scanline)
{
	scrollStates[scanline] = ScrollState();
}

// Reset the scroll history, but re-insert the most recent change
inline void VxlRawPPU::reset()
{
	ScrollState lastScrollState = scrollStates[mostRecentScanLineModified];
	scrollStates.clear();
	scrollStates[0] = lastScrollState;
}