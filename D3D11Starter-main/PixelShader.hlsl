
#include "ShaderIncludes.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

// Constant Buffer External Shader data
cbuffer ExternalData : register(b0)
{
	// Lighting related
    float4 ambient;
	
	// Material related
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;
    float roughness;
	
	// Camera related
    float3 cameraPosition;

	// Light
	Light directionalLight;
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
	
    input.normal = normalize(input.normal);

	// Create the surface color using the texture adjusted by the color tint
	float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);
	surfaceColor *= colorTint;
	
	// Utalize the ambient color
	float3 totalLight = surfaceColor * ambient;

	totalLight = DirectionalLight(directionalLight, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);

	return float4(totalLight, 1);
}