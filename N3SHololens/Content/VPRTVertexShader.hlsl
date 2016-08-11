struct Palette
{
	float3 hues[3];
};

struct PaletteCollection
{
	Palette palettes[8];
};

// A constant buffer that stores the model transform.
cbuffer ModelConstantBuffer : register(b0)
{
    float4x4 model;
};

// A constant buffer that stores each set of view and projection matrices in column-major format.
cbuffer ViewProjectionConstantBuffer : register(b1)
{
    float4x4 viewProjection[2];
};

cbuffer PaletteBuffer
{
	PaletteCollection palettes;
};

cbuffer PaletteSelectionBuffer
{
	int selectedPalette;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    min16float4 pos     : POSITION;
    min16float4 color   : COLOR0;
    uint        instId  : SV_InstanceID;
};

// Per-vertex data passed to the geometry shader.
// Note that the render target array index is set here in the vertex shader.
struct VertexShaderOutput
{
    min16float4 pos     : SV_POSITION;
    min16float4 color   : COLOR0;
    uint        rtvId   : SV_RenderTargetArrayIndex; // SV_InstanceID % 2
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 pos = input.pos;

    // Note which view this vertex has been sent to. Used for matrix lookup.
    // Taking the modulo of the instance ID allows geometry instancing to be used
    // along with stereo instanced drawing; in that case, two copies of each 
    // instance would be drawn, one for left and one for right.
    int idx = input.instId % 2;

    // Transform the vertex position into world space.
    pos = mul(pos, model);

    // Correct for perspective and project the vertex position onto the screen.
    pos = mul(pos, viewProjection[idx]);
    output.pos = (min16float4)pos;

	// Make the color X amount of the values of the palette as assigned by RGB input
	float4 hue = float4(0, 0, 0, 0);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[0].rgb, input.color.r);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[1].rgb, input.color.g);
	hue.rgb += mul(palettes.palettes[selectedPalette].hues[2].rgb, input.color.b);
	hue.a = input.color.a;
	output.color = hue;

    // Set the render target array index.
    output.rtvId = input.instId % 2;

    return output;
}
