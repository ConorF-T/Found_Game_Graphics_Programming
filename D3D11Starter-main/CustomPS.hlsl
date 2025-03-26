#include "ShaderIncludes.hlsli"

// Constant Buffer External Shader data
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
};

// Function for a random number
float random(float2 s)
{
	return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
};

// Function to create a special pattern
float2 truchetPattern(float2 s, float index)
{
	index = frac(((index - 0.5) * 2.0));
	if (index > 0.75)
	{
		s = float2(1, 1) - s;
	}
	else if (index > 0.5) 
	{
		s = float2(1.0 - s.x, s.y);
	}
	else if (index > 0.25) 
	{
		s = 1.0 - float2(1.0 - s.x, s.y);
	}
	return s;
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
	float2 st = input.screenPosition.xy / input.uv.xy;
	// Scalling everything to see the pattern better
	st *= 10.0f;

	float2 ipos = floor(st);
	float2 fpos = frac(st);

	float2 tile = truchetPattern(fpos, random(ipos));

	float colorFloat = 0.0f;

	colorFloat = smoothstep(tile.x - 0.3, tile.x, tile.y) - smoothstep(tile.x, tile.x + 0.3, tile.y);


	return float4(colorFloat, colorFloat, colorFloat, 1);
}