#include "stdafx.h"
#include "NesPPUSnapshot.h"

void VxlRawPPU::writeScrollValue(int v, int scanline) {
	// See if we need to add a new scroll state
	if (scrollWrites % 2 == 0)
	{
		scrollStates.push_back({ v, 0, scanline });
	}
	else 
	{
		scrollStates.back().y = v;
	}
	// Increment writes
	scrollWrites++;
}