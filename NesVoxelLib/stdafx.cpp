// stdafx.cpp : source file that includes just the standard includes
// NesVoxelLib.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


bool getBit(char data, int bit)
{
	if (bit > 7)
		bit = 7;
	else if (bit < 0)
		bit = 0;
	return (data >> bit) & 1;
}