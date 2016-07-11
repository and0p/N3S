struct Palette
{
	float3 hues[3];
};

struct PaletteCollection
{
	Palette palettes[8];
};

cbuffer PaletteBuffer
{
	PaletteCollection palettes;
};

cbuffer PaletteSelectionBuffer
{
	int selectedPalette;
};

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float4 main(float4 position : SV_POSITION, float4 color : COLOR, float3 normal : NORMAL) : SV_TARGET
{
	float4 hue = float4(0, 0, 0, 0);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[0].rgb, color.r);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[1].rgb, color.g);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[2].rgb, color.b);
	hue.a = color.a;
	return hue;
}