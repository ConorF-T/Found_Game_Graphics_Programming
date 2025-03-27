#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition    : POSITION; // XYZ position
    float2 uv               : TEXCOORD; // UV map
    float3 normal           : NORMAL; // Normal map
};

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition   : SV_POSITION; // XYZW position (System Value Position)
    float4 color            : COLOR; // RGBA color
    float2 uv               : TEXCOORD; // UV map
    float3 normal           : NORMAL; // normal map
    float3 worldPosition    : POSITION; // world position
};

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2
#define MAX_SPECULAR_EXPONENT 256.0f

struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float Intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding;
};

float SpecularPhong(float3 cameraPosition, float3 worldPosition, float3 incomingLightDirection, float3 normal, float roughness)
{
    // Calculate the specular exponent
    float specExponent = (1.0f – roughness) * MAX_SPECULAR_EXPONENT;

    // Calculate the view vector
    float3 viewVector = normalize(cameraPosition - worldPosition);

    // Calculate the reflection
    float3 reflection = reflect(incomingLightDirection, normal);

    // Calculate and return the specular value
    return(pow(max(dot(reflection, viewVector), 0.0f), specExponent));
}

float3 DirectionalLight(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness)
{
    // Calculate normalized direction to this light
    float3 directionToLight = normalize(-light.Direction);

    // Calculate the diffuse color
    float diffuseColor = saturate(dot(normal, directionToLight));

    // Calculate the specular
    float specular = SpecularPhong(cameraPos, worldPos, directionToLight, normal);

    return (diffuseColor * surfaceColor + specular) * light.Intensity * light.Color;
}


#endif