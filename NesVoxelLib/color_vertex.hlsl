struct MirrorState
{
	int x;
	int y;
};

cbuffer WorldMatrixBuffer
{
	matrix worldMatrix;
};
cbuffer ViewMatrixBuffer
{
	matrix viewMatrix;
};
cbuffer ProjectionMatrixBuffer
{
	matrix projectionMatrix;
};

cbuffer MirrorBuffer
{
	MirrorState mirrorState;
};

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

VOut main(float4 position : POSITION, float4 color : COLOR)
{
	VOut output;
	position.x *= mirrorState.x;
	position.y *= mirrorState.y;
	output.position = mul(position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	float4 hue = float4(0, 0, 0, 1);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[0].rgb, color.r);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[1].rgb, color.g);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[2].rgb, color.b);
	output.color = hue;
	return output;
}