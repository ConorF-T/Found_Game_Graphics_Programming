#pragma once

#include "SimpleShader.h"
#include "Camera.h"
#include "Transform.h"

#include <unordered_map>

class Material
{
public:
	Material(
		const char* name,
		DirectX::XMFLOAT4 colorTint,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		std::shared_ptr<SimplePixelShader> pixelShader,
		DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
		DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0)
		);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT2 GetuvScale();
	DirectX::XMFLOAT2 GetuvOffset();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTextureSRVs();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>& GetSamplers();
	float GetRoughness();
	const char* GetName();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 color);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vs);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> ps);
	void SetuvScale (DirectX::XMFLOAT2 scale);
	void SetuvOffset(DirectX::XMFLOAT2 offset);
	void SetRoughness(float r);

	void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);

	// Texture Business
	void AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string samplerName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	const char* name;

	// Hash Tables
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};