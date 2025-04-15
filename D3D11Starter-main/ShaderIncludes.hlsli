#ifndef __GGP_SHADER_INCLUDES__
#define __GGP_SHADER_INCLUDES__

// A constant Fresnel value for non-metals (glass and plastic have values of about 0.04)
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;

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

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}

// Calculates diffuse amount based on energy conservation
//
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
    // Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

    // ((n dot h)^2 * (a^2 - 1) + 1)
    // Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

    // Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
    // Pre-calculations
    float VdotH = saturate(dot(v, h));

    // Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
    // End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

    // Final value
    // Note: Numerator should be NdotV (or NdotL depending on parameters).
    // However, these are also in the BRDF's denominator, so they'll cancel!
    // We're leaving them out here AND in the BRDF function as the
    // dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
    // Other vectors
    float3 h = normalize(v + l);

    // Run numerator functions
    float  D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float  G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);

    // Pass F out of the function for diffuse balance
    F_out = F;

    // Final specular formula
    // Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
    // canceled out by our G() term.  As such, they have been removed
    // from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

    // One last non-obvious requirement: According to the rendering equation,
    // specular must have the same NdotL applied as diffuse!  We'll apply
    // that here so that minimal changes are required elsewhere.
    return specularResult * max(dot(n, l), 0);
}

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

float3 DirectionalLightPBR(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 specular)
{
    // Calculate normalized direction to this light
    float3 directionToLight = normalize(-light.Direction);

    // Get vector for the camera
    float3 vectorToCam = normalize(cameraPos - worldPos);

    // Calculate the light amounts
    float diff = DiffusePBR(normal, directionToLight);
    float3 F;
    float3 spec = MicrofacetBRDF(normal, directionToLight, vectorToCam, roughness, specular, F);

    // Calculate diffuse with energy conservation, including cutting diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);

    // Combine the final diffuse and specular values for this light
    return ( float4(balancedDiff, 1.0f) * surfaceColor, 1.0f + spec) * light.Intensity * light.Color;
}

float3 DirectionalLight(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness)
{
    // Calculate normalized direction to this light
    float3 directionToLight = normalize(-light.Direction);

    // Get vector for the camera
    float3 vectorToCam = normalize(cameraPos - worldPos);
    
    // Get diffusion
    float diffusion = saturate(dot(normal, directionToLight));
    
    // Get specular
    float specular = SpecularPhong(cameraPos, worldPos, directionToLight, normal, roughness);
    
    return (diffusion * surfaceColor + specular, 1.0f) * light.Intensity * light.Color;
}

float3 PointLightPBR(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 specular)
{
    // Get out vector for the position of the light
    float3 vectorToLight = normalize(light.Position - worldPos);

    // Get vector for the camera
    float3 vectorToCam = normalize(cameraPos - worldPos);

    // Calculate the light amounts
    float diff = DiffusePBR(normal, vectorToLight);
    float3 F;
    float3 spec = MicrofacetBRDF(normal, vectorToLight, vectorToCam, roughness, specular, F);

    // Calculate diffuse with energy conservation, including cutting diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diff, F, metalness);

    // Calculate the attenuation
    float attenuation = Attenuate(light, worldPos);

    return (float4(balancedDiff, 1.0f) * surfaceColor, 1.0f + spec) * attenuation * light.Intensity * light.Color;
}

float3 PointLight(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness)
{
    // Get out vector for the position of the light
    float3 vectorToLight = normalize(light.Position - worldPos);

    // Get vector for the camera
    float3 vectorToCam = normalize(cameraPos - worldPos);
    
    // Get attenuation
    float dist = distance(light.Position, worldPos);
    float attenuation = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    
    // Get specular
    float specular = SpecularPhong(cameraPos, worldPos, vectorToLight, normal, roughness);
    
    // Get diffusion
    float diffusion = saturate(dot(normal, vectorToLight));
    
    return (diffusion * surfaceColor + specular, 1.0f) * attenuation * light.Intensity * light.Color;
}

float3 SpotLightPBR(Light light, float3 normal, float4 surfaceColor, float3 cameraPos, float3 worldPos, float roughness, float metalness, float3 specular)
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

    return PointLightPBR(light, normal, surfaceColor, cameraPos, worldPos, roughness, metalness, specular) * spotTerm;
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