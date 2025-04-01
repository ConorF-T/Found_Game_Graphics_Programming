#pragma once

#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

#include <memory>
#include <wrl/client.h>

class Sky
{
public:
	Sky(
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions,
		std::shared_ptr<Mesh> mesh,
		std::shared_ptr<SimplePixelShader> skyPS,
		std::shared_ptr<SimpleVertexShader> skyVS,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back
	);

	// Draw function
	void Draw(std::shared_ptr<Camera> camera);

	// Getter
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetCubeMap();

private:
	// Cube Map
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMap;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;

	// Shaders
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> skyVS;

	// Mesh
	std::shared_ptr<Mesh> mesh;

	// Sampler Options
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};

