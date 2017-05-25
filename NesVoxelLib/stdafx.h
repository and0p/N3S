// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0600
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

#include <wrl/client.h>

// TODO: Move these to more specific headers? N3s3d should be only code calling raw DX
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

bool getBit(char data, int bit);

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception();
		}
	}
}

// Simple utility functions

inline bool toggleBool(bool b)
{
	if (b == false)
		return true;
	else
		return false;;
}

struct Vector2D {
	float x;
	float y;
};



inline int getArrayIndexFromXY(int x, int y, int arrayWidth)
{
	return (y * arrayWidth) + x;
}

inline Vector2D unwrapArrayIndex(int i, int arrayWidth)
{
	float y = floor(i / arrayWidth);
	return { (float)(i % arrayWidth), y };
}

inline int roundDownPosOrNeg(float n)
{
	if (n < 0)
		return ceil(n);
	else
		return floor(n);
}

// Snap to nearest nametable point
inline Vector2D nametableSnap(int x, int y)
{
	float xDiv = x / 8;
	float yDiv = y / 8;
	xDiv += 0.5f;
	yDiv += 0.5f;
	return { floor(xDiv) * 8, floor(yDiv) * 8 };
}