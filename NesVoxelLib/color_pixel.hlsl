struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float4 main(float4 position : SV_POSITION, float4 color : COLOR, float3 normal : NORMAL) : SV_TARGET
{
	return color;
}