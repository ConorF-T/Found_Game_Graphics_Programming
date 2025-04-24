#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Vectors to hold our meshes and entities
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector <std::shared_ptr<Material>> materials;

	// Game Camera
	std::vector<std::shared_ptr<Camera>> cameras;
	float cameraMoveSpeed = 3.0f;
	float cameraSensativity = 0.001f;

	// Window Color
	float color[4] = {0.4f, 0.6f, 0.75f, 0.0f};

	// Ambient Color
	DirectX::XMFLOAT4 ambientColor;

	// Lights
	std::vector<Light>  lights;

	// Sky box
	std::shared_ptr<Sky> sky;

	// Shadow Mapping reseources
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;

	float shadowMapResolution;

	std::shared_ptr<SimpleVertexShader> shadowVS;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> fullscreenVS;

	// Resources that are tied to a particular post process
	std::shared_ptr<SimplePixelShader> boxBlurPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // For sampling

	int blurRadius = 100;
};

