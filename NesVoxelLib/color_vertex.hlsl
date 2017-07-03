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
	int selectedPalette;
};

cbuffer CameraPosition
{
	float4 cameraPos;
};

struct VOut
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
	float dist : FOG;
};

VOut main(float4 position : POSITION, float2 uv : TEXCOORD, int color : COLOR)
{
	VOut output;
	// Mirror vertex, if sprite needed
	position.x *= mirrorState.x;
	position.y *= mirrorState.y;
	// World transform
	output.position = mul(position, worldMatrix);
	// Get distance between eye position and vertex in world-space
	output.dist = distance(cameraPos, output.position) * 0.6;
	// View + Projection transform
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	// Get palette color
	float4 hue = float4(0, 0, 0, 1);
	hue.rgb = palettes.palettes[selectedPalette].hues[color];
	output.color = hue;
	output.uv = uv;
	// Return output
	return output;
}