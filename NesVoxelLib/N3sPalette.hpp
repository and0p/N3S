#pragma once

struct Hue
{
	float red;
	float green;
	float blue;
};

struct HueSet
{
	Hue hues[64];
};

struct PPUHueStandard
{
	HueSet hueSets[8];
};

enum PPUType { v2C02, v2C03, v2C05 };

class N3sPalette
{
public:
	N3sPalette();
	N3sPalette(int colors[25]);
	int colorIndices[24];		// All palette colors that are not the background
	int backgroundColorIndex;	// Background color
	void updateShaderPalette();
	Hue getBackgroundColor();
	static Hue getHue(int color);
	static void setPPUType(PPUType type);
	static void init();
private:
	static PPUType currentPPUType;
	static PPUHueStandard standards[3];
};