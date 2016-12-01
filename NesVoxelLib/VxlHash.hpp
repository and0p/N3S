#pragma once
#include "stdafx.h"
using namespace std;

// Walk along a series of bytes, appending char at every step to string
inline string simpleHash(char* data, int size, int depth)
{
	if (depth > size)
		depth = size;
	string s = "";
	int step = size / depth;
	for (int i = 0; i < depth; i++)
	{
		s.append(&data[step * i]);
	}
	return s;
}