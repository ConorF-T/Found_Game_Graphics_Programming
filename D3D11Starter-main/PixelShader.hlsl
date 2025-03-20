
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float4 color			: COLOR;
	float2 uv				: TEXCOORD;		// UV map
	float3 normal			: NORMAL;		// normal map
};

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

// Constant Buffer External Shader data
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Set the uv adjusting with the scale and offset
	input.uv = input.uv * uvScale + uvOffset;

	// Create the surface color using the texture adjusted by the color tint
	float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);

	surfaceColor *= colorTint;

	return float4(surfaceColor);
}