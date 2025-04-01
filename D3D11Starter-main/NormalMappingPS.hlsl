#include "ShaderIncludes.hlsli"

cbuffer ExternalData : register(b0)
{
	// Material related
	float4 colorTint;
	float2 uvScale;
	float2 uvOffset;
	float roughness;

	// Camera related
	float3 cameraPosition;

	// Lighting related
	float4 ambient;

	// Light
	Light lights[5];
	int  lightCount;
}

Texture2D SurfaceTexture	: register(t0); // "t" registers for textures
Texture2D NormalMap			: register(t1);

SamplerState BasicSampler	: register(s0); // "s" registers for samplers

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
	// Make sure they're normalized
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Unpack the normal map
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal); // Don’t forget to normalize!

	// Create our TBN matrix with our normalized normal and tanegnt
	float3 N = input.normal;
	float3 T = input.tangent;
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	// Transform the unpacked normal
	input.normal = mul(unpackedNormal, TBN); // Note multiplication order!

	// Set the uv adjusting with the scale and offset
	input.uv = input.uv * uvScale + uvOffset;

	input.normal = normalize(input.normal);

	// Create the surface color using the texture adjusted by the color tint
	float4 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv);
	surfaceColor *= colorTint;

	// Utalize the ambient color
	float3 totalLight = surfaceColor * ambient;


	//  Loop  through all  the lights and calculate the light
	for (int i = 0; i < lightCount; i++)
	{
		// Get our light ready
		Light l = lights[i];
		l.Direction = normalize(l.Direction);

		// Switch statement for the different types of light possible in the list
		switch (l.Type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			totalLight += DirectionalLight(l, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);
			break;

		case LIGHT_TYPE_POINT:
			totalLight += PointLight(l, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);
			break;

		case LIGHT_TYPE_SPOT:
			totalLight += SpotLight(l, input.normal, surfaceColor, cameraPosition, input.worldPosition, roughness);
			break;
		}
	}

	return float4(totalLight, 1);
}