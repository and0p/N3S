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

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float4 normal : NORMAL;
};

VOut main(float4 position : POSITION, float4 color : COLOR, float4 normal : NORMAL)
{
	VOut output;
	position.x *= mirrorState.x;
	position.y *= mirrorState.y;
	output.position = mul(position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.color = color;
	output.normal = normal;
	return output;
}