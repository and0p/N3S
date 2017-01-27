#include "stdafx.h"
#include "N3sPalette.hpp"
#include "N3s3d.hpp"

PPUType N3sPalette::currentPPUType;
PPUHueStandard N3sPalette::standards[3];

float getFloatColorValue(int color)
{
	if (color <= 0)
		return 0.0f;
	else if (color >= 255)
		return 1.0f;
	else
		return color / 256.0f;
}

void N3sPalette::init()
{
	currentPPUType = v2C02;

	int hueArray[192] = { 84,84,84,0,30,116,8,16,144,48,0,136,68,0,100,92,0,48,84,4,0,60,24,0,32,42,0,8,58,0,0,64,0,0,60,0,0,50,60,5,5,5,0,5,5,5,0,0,152,150,152,8,76,196,48,50,236,92,30,228,136,20,176,160,20,100,152,34,32,120,60,0,84,90,0,40,114,0,8,124,0,0,118,40,0,102,120,5,5,5,0,5,5,5,0,0,236,238,236,76,154,236,120,124,236,176,98,236,228,84,236,236,88,180,236,106,100,212,136,32,160,170,0,116,196,0,76,208,32,56,204,108,56,180,204,60,60,60,5,5,5,5,5,5,236,238,236,168,204,236,188,188,236,212,178,236,236,174,236,236,174,212,236,180,176,228,196,144,204,210,120,180,222,120,168,226,144,152,226,180,160,214,228,160,162,160,5,5,5,5,5,5 };
	for (int h = 0; h < 64; h++)
	{
		standards[0].hueSets[0].hues[h].red = getFloatColorValue(hueArray[h * 3]);
		standards[0].hueSets[0].hues[h].green = getFloatColorValue(hueArray[h * 3 + 1]);
		standards[0].hueSets[0].hues[h].blue = getFloatColorValue(hueArray[h * 3 + 2]);
	}
}

void N3sPalette::updateShaderPalette()
{
	N3s3d::setShader(color);
	float colors[72];
	for (int i = 0; i < 24; i++)
	{
		int pos = i * 3;
		colors[pos] = standards[currentPPUType].hueSets[0].hues[colorIndices[i]].red;
		colors[pos + 1] = standards[currentPPUType].hueSets[0].hues[colorIndices[i]].green;
		colors[pos + 2] = standards[currentPPUType].hueSets[0].hues[colorIndices[i]].blue;
	}
	N3s3d::updatePalette(&colors[0]);
}

Hue N3sPalette::getBackgroundColor()
{
	return  standards[currentPPUType].hueSets[0].hues[backgroundColorIndex];
}

void N3sPalette::setPPUType(PPUType type)
{
	currentPPUType = type;
}

N3sPalette::N3sPalette()
{
	backgroundColorIndex = 0;
	for (int i = 0; i < 24; i++)
	{
		colorIndices[i] = i + 1;
	}
}


N3sPalette::N3sPalette(int colors[25])
{
	backgroundColorIndex = colors[0];
	for (int i = 0; i < 24; i++)
	{
		colorIndices[i] = colors[i+1];
	}
}
