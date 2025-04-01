#include "Sky.h"

Sky::Sky(
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions, 
	std::shared_ptr<Mesh> mesh, 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMap, 
	std::shared_ptr<SimplePixelShader> skyPS, 
	std::shared_ptr<SimpleVertexShader> skyVS)
	:
	samplerOptions(samplerOptions),

{
}

void Sky::Draw(std::shared_ptr<Camera> camera)
{
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetCubeMap()
{
	return Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();
}
