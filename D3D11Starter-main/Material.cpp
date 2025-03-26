#include "Material.h"

// Constructor
Material::Material(
	const char* name,
	DirectX::XMFLOAT4 colorTint, 
	std::shared_ptr<SimpleVertexShader> vertexShader, 
	std::shared_ptr<SimplePixelShader> pixelShader, 
	float roughness,
	DirectX::XMFLOAT2 uvScale, 
	DirectX::XMFLOAT2 uvOffset)
	:
	name(name),
	colorTint(colorTint),
	vertexShader(vertexShader),
	pixelShader(pixelShader),
	roughness(roughness),
	uvScale(uvScale),
	uvOffset(uvOffset)
{

}


// Getters
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertexShader; }

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }

DirectX::XMFLOAT2 Material::GetuvScale()
{
	return DirectX::XMFLOAT2();
}

DirectX::XMFLOAT2 Material::GetuvOffset()
{
	return DirectX::XMFLOAT2();
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& Material::GetTextureSRVs()
{
	return textureSRVs;
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>& Material::GetSamplers()
{
	return samplers;
}

float Material::GetRoughness()
{
	return 0.0f;
}


// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 color) { this->colorTint = color; }

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vs) { this->vertexShader = vs; }

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> ps) { this->pixelShader = ps; }

void Material::SetuvScale(DirectX::XMFLOAT2 scale) { this->uvScale = scale; }

void Material::SetuvOffset(DirectX::XMFLOAT2 offset) { this->uvOffset = offset; }

void Material::SetRoughness(float r) { this->roughness = r; }


// Texture Business
void Material::AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ shaderVariableName, srv });
}

void Material::AddSampler(std::string samplerName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ samplerName, sampler });
}

// Prepares the shaders and texture srv
void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	// Constant Buffer Business
	std::shared_ptr<SimpleVertexShader> vs = vertexShader;
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vs->CopyAllBufferData();

	// More constant buffer business with the pixel shader this time
	std::shared_ptr<SimplePixelShader> ps = pixelShader;
	ps->SetFloat4("colorTint", colorTint);
	ps->SetFloat2("uvScale", uvScale);
	ps->SetFloat2("uvOffset", uvOffset);
	ps->SetFloat("roughness", roughness);
	ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
	ps->CopyAllBufferData();

	// Activate our shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	// Texture hash tables
	for (auto& t : textureSRVs) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { ps->SetSamplerState(s.first.c_str(), s.second); }
}
