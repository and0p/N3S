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
	float3 hues[4];
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
	int selection[2];
};

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VOut main(float4 position : POSITION)
{
	VOut output;
	// Mirror vertex, if sprite needed
	position.x *= mirrorState.x;
	position.y *= mirrorState.y;
	// World transform
	output.position = mul(position, worldMatrix);
	// View + Projection transform
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	// Get palette color
	output.color.rgb = palettes.palettes[selection[0]].hues[selection[1]];
	output.color.a = 255;
	return output;
}