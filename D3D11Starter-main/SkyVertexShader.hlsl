#include "ShaderIncludes.hlsli"

// Constant Buffer External Shader Data
cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel_Sky main(VertexShaderInput input)
{
	VertexToPixel_Sky output;

	// Remove trasnlation from the view matrix
	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	// apply projection and updated view to the input position
	matrix viewProjection = mul(projection, viewNoTranslation);
	output.screenPosition = mul(viewProjection, float4(input.localPosition, 1.0f));

	// Ensures the output depth of each vertex is exactly 1.0
	output.screenPosition.z = output.screenPosition.w;

	output.sampleDir = input.localPosition;

	return output;
}