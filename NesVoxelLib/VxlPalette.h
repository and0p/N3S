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

class PPUHueStandardCollection
{
public:
	PPUHueStandardCollection();
	Hue getHue(PPUType type, int hueSet, int number);
private:
	PPUHueStandard ppuHueStandards[3];
};