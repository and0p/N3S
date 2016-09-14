#pragma once

#include <vector>
#include <map>

struct PatternTable {
	char data[8192];
};

struct NameTable {
	char data[1024];
};

struct ScrollAddress {
	int coarseX = 0;
	int coarseY = 0;
	int fineX = 0;
	int fineY = 0;
	int nametable = 0;
};

class ScrollSnapshot
{
public:
	bool patternSelect = 0;
	ScrollAddress t;
	ScrollAddress v;
	void poke(int reg, bool toggle, int data);
	bool tToV;
	void changeVScroll(int y);
	int getTrueX();
	int getTrueY();
};

inline void ScrollSnapshot::poke(int reg, bool toggle, int data)
{
	switch (reg) {
	case 2000:
		patternSelect = data >> 4 & 1;
		t.nametable = data & 3;
		v.nametable = data & 3; // Still curious about this behavior
		break;
	case 2005:
		if (!toggle)
		{
			t.fineX = data & 7;
			t.coarseX = data >> 3;
			// V technically also gets set by the time we're on the next scanline
			v.fineX = data & 7;
			v.coarseX = data >> 3;
		}
		else
		{
			t.fineY = data & 7;
			t.coarseY = data >> 3;
		}
		break;
	case 2006:
		if (!toggle)
		{
			// Low 2 bits overwrites high 2 bits of coarse Y
			int coarseYHigh = data & 3;
			coarseYHigh = coarseYHigh << 3;
			t.coarseY = coarseYHigh + (t.coarseY & 7);
			// Bits 2 & 3 become nametable
			t.nametable = (data >> 2) & 3;
			// Bits 4 & 5 becomes fine Y, bit 2 of fine Y is set to 0 by operation
			t.fineY = (data >> 4) & 3;
		}
		else
		{
			// Coarse X becomes bits 0-4
			t.coarseX = data & 31;
			// Low 3 bits of coarse Y becomes high 3 bits of data
			int high2Y = t.coarseY & 24;
			t.coarseY = (data >> 5 & 7) + high2Y;
			// Set flag to push T to V at end of scanline
			tToV = true;
		}
		break;
	}
	// if T-to-V is set, push every change
	if (tToV)
		v = t;
}

inline void ScrollSnapshot::changeVScroll(int y)
{
	int coarseYChange = floor(y / 8);
	int fineYChange = y % 8;
	v.coarseY += coarseYChange;
	v.fineY += fineYChange;
}

inline int ScrollSnapshot::getTrueX()
{
	return (v.coarseX * 8) + v.fineX;
}

inline int ScrollSnapshot::getTrueY()
{
	return (v.coarseY * 8) + v.fineY;
}

struct VxlRawPPU {
public:
	char ctrl;
	char mask;
	char status;
	char palette[32];
	char oam[256];
	int mirroring;
	PatternTable patternTable;
	NameTable nameTables[4];
	std::map<int, ScrollSnapshot> scrollSnapshots;
	std::map<int, bool> oamPatternSelect;
	void writeScrollValue(int scanline, int reg, bool toggle, int data);
	void setOAMPatternSelect(int scanline, bool select);
	void reset(int mirrorType, bool oamPattern, bool namePattern);
private:
	int mostRecentScanlineModified = 0;
	int mostRecentOamPatternScanline = 0;
};

inline void VxlRawPPU::writeScrollValue(int scanline, int reg, bool toggle, int data) {
	// Increment the scanline, assuming these changes are applying to next line
	scanline++;
	int i = 0;
	// Set scanline to 0 if we are outside of visible range, as it will all wrap back to rendering at 0
	if (scanline < 0 || scanline > 240)
		scanline = 0;
	// Add a new snapshot for this scanline if it doesn't exist yet
	if (scrollSnapshots.count(scanline) < 1)
	{
		// Copy the last one
		ScrollSnapshot newSnapshot = scrollSnapshots[mostRecentScanlineModified];
		newSnapshot.tToV = false;
		// Assume each scanline since has been incrementing the Y value
		newSnapshot.changeVScroll(scanline - mostRecentScanlineModified);
		// Set modified copy to current scanline and mark as most recent
		scrollSnapshots[scanline] = newSnapshot;
		mostRecentScanlineModified = scanline;
	}
	// Poke reg with data
	scrollSnapshots[scanline].poke(reg, toggle, data);
}

inline void VxlRawPPU::setOAMPatternSelect(int scanline, bool select) {
	// Increment the scanline, assuming these changes are applying to next line
	scanline++;
	// Set scanline to 0 if we are outside of visible range, as it will all wrap back to rendering at 0
	if (scanline < 0 || scanline > 240)
		scanline = 0;
	if (select != oamPatternSelect[mostRecentOamPatternScanline])
	{
		oamPatternSelect[scanline] = select;
		mostRecentOamPatternScanline = scanline;
	}
}

// Reset the scroll history, but re-insert the most recent snapshot and set v to t
inline void VxlRawPPU::reset(int mirrorType, bool oamPattern, bool namePattern)
{
	ScrollSnapshot rollover = scrollSnapshots[mostRecentScanlineModified];
	scrollSnapshots.clear();
	bool lastOAMPatternSelect = oamPatternSelect[mostRecentOamPatternScanline];
	oamPatternSelect.clear();
	oamPatternSelect[0] = mostRecentOamPatternScanline;
	rollover.v = rollover.t;
	scrollSnapshots[0] = rollover;
	scrollSnapshots[0];
	mirroring = mirrorType;
	mostRecentScanlineModified = 0;
}