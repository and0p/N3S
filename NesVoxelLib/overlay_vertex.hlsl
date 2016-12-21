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

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	float4 output;
	output = mul(pos, worldMatrix);
	output = mul(output, viewMatrix);
	output = mul(output, projectionMatrix);
	return output;
}