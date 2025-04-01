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
    float3 tangent          : TANGENT; // tangent
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
    float3 tangent          : TANGENT; // tangent
};

struct VertexToPixel_Sky
{
    // Data type
    //  |
    //  |   Name          Semantic
    //  |    |                |
    //  v    v                v
    float4 screenPosition	: SV_POSITION;
    float3 sampleDir		: DIRECTION;
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
    float specExponent = (1.0f - roughness) * 260;

    // Calculate the view vector
    float3 viewVector = normalize(cameraPosition - worldPosition);

    // Calculate the reflection
    float3 reflection = reflect(incomingLightDirection, normal);

    // Calculate and return the specular value
    return (roughness == 1 ? 0.0f : pow(max(dot(reflection, viewVector), 0.0f), specExponent));
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

float3 DirectionalLight(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness)
{
    // Calculate normalized direction to this light
    float3 directionToLight = normalize(-light.Direction);

    // Calculate the diffuse color
    float diffuseColor = saturate(dot(normal, directionToLight));

    // Calculate the specular
    float specular = SpecularPhong(cameraPos, worldPos, directionToLight, normal, roughness);

    return (diffuseColor * surfaceColor + specular) * light.Intensity * light.Color;
}

float3 PointLight(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness)
{
    // Get out vector for the position of the light
    float3 vectorToLight = normalize(light.Position - worldPos);

    // Calculate the diffusion color
    float diffuseColor = saturate(dot(normal, vectorToLight));

    // Calculate teh attenuation
    float attenuation = Attenuate(light, worldPos);

    // Calculate the specular
    float specular = SpecularPhong(cameraPos, worldPos, vectorToLight, normal, roughness);

    return (diffuseColor * surfaceColor + specular) * attenuation * light.Intensity * light.Color;
}

float3 SpotLight(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness)
{
    // Get out vector for the position of the light
    float3 vectorToLight = normalize(light.Position - worldPos);

    // Get cos(angle) between the pixel and the light's direction
    float pixelAngle = saturate(dot(vectorToLight, light.Direction));

    // Get the cosine of the angles and our outter range
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;

    // Linear falloff over the range, clamp 0-1, apply to light calculation
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);

    return PointLight(light, normal, surfaceColor, cameraPos, worldPos, roughness) * spotTerm;
}


#endif