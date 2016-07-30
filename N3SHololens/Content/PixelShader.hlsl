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

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	min16float4 pos   : SV_POSITION;
	min16float4 color : COLOR0;
};

// The pixel shader passes through the color data. The color data from 
// is interpolated and assigned to a pixel at the rasterization step.
min16float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 hue = float4(0, 0, 0, 0);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[0].rgb, input.color.r);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[1].rgb, input.color.g);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[2].rgb, input.color.b);
	hue.a = 1;
	return hue;
}
