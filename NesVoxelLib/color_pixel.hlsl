float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD, float4 color : COLOR, float dist : FOG) : SV_TARGET
{
	float wireframe =	smoothstep(0.9, 0.95, uv.x) +
						(1 - smoothstep(0.05, 0.1, uv.x)) +
						smoothstep(0.9, 0.95, uv.y) +
						(1 - smoothstep(0.05, 0.1, uv.y));
	color.rgb -= saturate(clamp(0, 1, wireframe) *  (1 - dist)) * 0.2;
	return color;
}