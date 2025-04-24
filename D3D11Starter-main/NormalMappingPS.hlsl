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

// Texture business
Texture2D Albedo						: register(t0);
Texture2D NormalMap						: register(t1);
Texture2D RoughnessMap					: register(t2);
Texture2D MetalnessMap					: register(t3);
Texture2D ShadowMap						: register(t4);
SamplerState BasicSampler				: register(s0);
SamplerComparisonState ShadowSampler	: register(s1);


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
	float4 surfaceColor = pow( Albedo.Sample(BasicSampler, input.uv), 2.2f);
	//surfaceColor *= colorTint;

	// Grab the roughness from the roughness map's red channel
	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;

	// Grab the metalness from the metal map's red channel
	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

	// Perform the perspective divide (divide by W) ourselves
	input.shadowMapPos /= input.shadowMapPos.w;

	// Convert the normalized device coordinates to UVs for sampling
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y; // Flip the Y

	// Grab the distances we need: light-to-pixel and closest-surface
	float distToLight = input.shadowMapPos.z;
	float distShadowMap = ShadowMap.Sample(BasicSampler, shadowUV).r;

	// Get a ratio of comparison results using SampleCmpLevelZero()
	float shadowAmount = ShadowMap.SampleCmpLevelZero(
		ShadowSampler,
		shadowUV,
		distToLight).r;

	// Create our total light
	float3 totalLight = surfaceColor.rgb * ambient;

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
			float3 lightResult = DirectionalLightPBR(l, input.normal, surfaceColor.rgb, cameraPosition, input.worldPosition, roughness, metalness, specularColor);
			if (i == 0)
			{
				// Apply the shadowing result
				lightResult *= shadowAmount;

			}
			// Add this light's result to the total light for this pixel
			totalLight += lightResult;
			break;

		case LIGHT_TYPE_POINT:
			totalLight += PointLightPBR(l, input.normal, surfaceColor.rgb, cameraPosition, input.worldPosition, roughness, metalness, specularColor);
			break;

		case LIGHT_TYPE_SPOT:
			totalLight += SpotLightPBR(l, input.normal, surfaceColor.rgb, cameraPosition, input.worldPosition, roughness, metalness, specularColor);
			break;
		}
	}

	return float4(pow(totalLight, 1.0f / 2.2f), 1);
}