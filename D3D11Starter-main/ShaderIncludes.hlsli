#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD; // UV map
    float3 normal : NORMAL; // Normal map
};

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float4 color : COLOR; // RGBA color
    float2 uv : TEXCOORD; // UV map
    float3 normal : NORMAL; // normal map
    float3 worldPosition : POSITION; // world position
};


#endif