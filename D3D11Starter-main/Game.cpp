#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "Transform.h"
#include "WICTextureLoader.h"
#include <memory>

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Create our new camera
	std::shared_ptr<Camera> gameCamera = std::make_shared<Camera>(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XM_PIDIV4,
		Window::AspectRatio(),
		1.0f,
		100.0f,
		true,
		cameraMoveSpeed,	// Will be able to be changed with a UI element (hopefully)
		cameraSensativity	// Same thing here (hopefully)
		);

	// Set the camera to be active
	gameCamera->SetActive(true);
	cameras.push_back(gameCamera);

	// Create our second camera
	std::shared_ptr<Camera> altCamera = std::make_shared<Camera>(
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XM_PIDIV4,
		Window::AspectRatio(),
		1.0f,
		100.0f,
		true,
		cameraMoveSpeed,	// Will be able to be changed with a UI element (hopefully)
		cameraSensativity	// Same thing here (hopefully)
	);

	// Set the alt camera to inactive
	altCamera->SetActive(false);
	cameras.push_back(altCamera);
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	// Load our shaders
	std::shared_ptr<SimpleVertexShader> vertexShader = std::make_shared<SimpleVertexShader>(		// Basic Vertex Shader
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = std::make_shared<SimplePixelShader>(			// Basic Pixel Shader
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> uvPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> customPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> DecalPixelShader = std::make_shared<SimplePixelShader>(		// Shader with 2 layered textures
		Graphics::Device, Graphics::Context, FixPath(L"DecalPixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalMappingPS = std::make_shared<SimplePixelShader>(		// Pixel Shader with normal mapping
		Graphics::Device, Graphics::Context, FixPath(L"NormalMappingPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> skyPixelShader = std::make_shared<SimplePixelShader>(		// Pixel shader for the skybox
		Graphics::Device, Graphics::Context, FixPath(L"SkyPixelShader.cso").c_str());
	std::shared_ptr<SimpleVertexShader> skyVertexShader = std::make_shared<SimpleVertexShader>(		// Vertex shader for the skybox
		Graphics::Device, Graphics::Context, FixPath(L"SkyVertexShader.cso").c_str());
	shadowVS = std::make_shared<SimpleVertexShader>( Graphics::Device, Graphics::Context, FixPath(L"ShadowMapVS.cso").c_str() ); // Vertex shader for shadow mapping

	// Post Processing
	boxBlurPS = std::make_shared<SimplePixelShader>(		// Pixel shader handling the box blur post processing effect
		Graphics::Device, Graphics::Context, FixPath(L"BoxBlurPixelShader.cso").c_str());
	fullscreenVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"FullscreenVerexShader.cso").c_str());

	shadowMapResolution = 1024.0f;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Set up the shadow sampler state for comparison
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Rasterizer state for depth biasing
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Window::Width();
	textureDesc.Height = Window::Height();
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());

	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());

	// Load some textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/brick_texture.jpg").c_str(), nullptr, brickSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sandSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/sand_texture.jpg").c_str(), nullptr, sandSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dirtSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/dirt_texture.png").c_str(), nullptr, dirtSRV.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/cobblestone.png").c_str(), nullptr, cobbleSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/cushion.png").c_str(), nullptr, cushionSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/rock.png").c_str(), nullptr, rockSRV.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeAlbedo, bronzeNormal, bronzeMetal, bronzeRoughness;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Albedos/bronze_albedo.png").c_str(), nullptr, bronzeAlbedo.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/bronze_metal.png").c_str(), nullptr, bronzeMetal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/bronze_normals.png").c_str(), nullptr, bronzeNormal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Roughness/bronze_roughness.png").c_str(), nullptr, bronzeRoughness.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintAlbedo, paintNormal, paintMetal, paintRoughness;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Albedos/paint_albedo.png").c_str(), nullptr, paintAlbedo.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/paint_metal.png").c_str(), nullptr, paintMetal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/paint_normals.png").c_str(), nullptr, paintNormal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Roughness/paint_roughness.png").c_str(), nullptr, paintRoughness.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorAlbedo, floorNormal, floorMetal, floorRoughness;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Albedos/floor_albedo.png").c_str(), nullptr, floorAlbedo.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/floor_metal.png").c_str(), nullptr, floorMetal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/floor_normals.png").c_str(), nullptr, floorNormal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Roughness/floor_roughness.png").c_str(), nullptr, floorRoughness.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodAlbedo, woodNormal, woodMetal, woodRoughness;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Albedos/wood_albedo.png").c_str(), nullptr,woodAlbedo.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Metal/wood_metal.png").c_str(), nullptr, woodMetal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/wood_normals.png").c_str(), nullptr, woodNormal.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Roughness/wood_roughness.png").c_str(), nullptr, woodRoughness.GetAddressOf());

	// Normal Maps
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleNormalsSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/cobblestone_normals.png").c_str(), nullptr, cobbleNormalsSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionNormalsSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/cushion_normals.png").c_str(), nullptr, cushionNormalsSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormalsSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/Normals/rock_normals.png").c_str(), nullptr, rockNormalsSRV.GetAddressOf());

	// Sampler State stuff
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;		// Give us the best for angles
	samplerDesc.MaxAnisotropy = 8;						// Setting this to medium because idk exaclty how it effects things
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Actually Create it
	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

	// Create our materials
	std::shared_ptr<Material> matBricks = std::make_shared<Material>("Bricks", white, vertexShader, pixelShader);
	matBricks->AddSampler("BasicSampler", samplerState);
	matBricks->AddTextureSRV("SurfaceTexture", brickSRV);

	std::shared_ptr<Material> matSand = std::make_shared<Material>("Sand", white, vertexShader, pixelShader);
	matSand->AddSampler("BasicSampler", samplerState);
	matSand->AddTextureSRV("SurfaceTexture", sandSRV);

	std::shared_ptr<Material> matDirtBricks = std::make_shared<Material>("Dirty Bricks", white, vertexShader, DecalPixelShader);
	matDirtBricks->AddSampler("BasicSampler", samplerState);
	matDirtBricks->AddTextureSRV("SurfaceTexture", brickSRV);
	matDirtBricks->AddTextureSRV("DecalTexture", dirtSRV);

	// Materials with normal maps
	std::shared_ptr<Material> cobbleMat = std::make_shared<Material>("Cobblestone", white, vertexShader, normalMappingPS);
	cobbleMat->AddSampler("BasicSampler", samplerState);
	cobbleMat->AddTextureSRV("SurfaceTexture", cobbleSRV);
	cobbleMat->AddTextureSRV("NormalMap", cobbleNormalsSRV);

	std::shared_ptr<Material> cushionMat = std::make_shared<Material>("Cushion", white, vertexShader, normalMappingPS);
	cushionMat->AddSampler("BasicSampler", samplerState);
	cushionMat->AddTextureSRV("SurfaceTexture", cushionSRV);
	cushionMat->AddTextureSRV("NormalMap", cushionNormalsSRV);

	std::shared_ptr<Material> rockMat = std::make_shared<Material>("Rock", white, vertexShader, normalMappingPS);
	rockMat->AddSampler("BasicSampler", samplerState);
	rockMat->AddTextureSRV("SurfaceTexture", rockSRV);
	rockMat->AddTextureSRV("NormalMap", rockNormalsSRV);

	// PBR Materials
	std::shared_ptr<Material> bronzeMat = std::make_shared <Material> ("Bronze", white, vertexShader, normalMappingPS);
	bronzeMat->AddSampler("BasicSampler", samplerState);
	bronzeMat->AddTextureSRV("Albedo", bronzeAlbedo);
	bronzeMat->AddTextureSRV("NormalMap", bronzeNormal);
	bronzeMat->AddTextureSRV("RoughnessMap", bronzeRoughness);
	bronzeMat->AddTextureSRV("MetalnessMap", bronzeMetal);

	std::shared_ptr<Material> paintMat = std::make_shared <Material>("Paint", white, vertexShader, normalMappingPS);
	paintMat->AddSampler("BasicSampler", samplerState);
	paintMat->AddTextureSRV("Albedo", paintAlbedo);
	paintMat->AddTextureSRV("NormalMap", paintNormal);
	paintMat->AddTextureSRV("RoughnessMap", paintRoughness);
	paintMat->AddTextureSRV("MetalnessMap", paintMetal);

	std::shared_ptr<Material> floorMat = std::make_shared <Material>("Floor", white, vertexShader, normalMappingPS);
	floorMat->AddSampler("BasicSampler", samplerState);
	floorMat->AddTextureSRV("Albedo", floorAlbedo);
	floorMat->AddTextureSRV("NormalMap", floorNormal);
	floorMat->AddTextureSRV("RoughnessMap", floorRoughness);
	floorMat->AddTextureSRV("MetalnessMap", floorMetal);

	std::shared_ptr<Material> woodMat = std::make_shared <Material>("Wood", white, vertexShader, normalMappingPS);
	woodMat->AddSampler("BasicSampler", samplerState);
	woodMat->AddTextureSRV("Albedo", woodAlbedo);
	woodMat->AddTextureSRV("NormalMap", woodNormal);
	woodMat->AddTextureSRV("RoughnessMap", woodRoughness);
	woodMat->AddTextureSRV("MetalnessMap", woodMetal);


	// Add materials to vector
	materials.insert(materials.end(), { matBricks, matSand, matDirtBricks, cobbleMat, cushionMat, rockMat, bronzeMat, paintMat, floorMat, woodMat});

	// Create our meshes with .obj files
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("Cube", FixPath("../../Assets/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("Helix", FixPath("../../Assets/helix.obj").c_str());
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>("Quad", FixPath("../../Assets/quad.obj").c_str());
	std::shared_ptr<Mesh> dubQuadMesh = std::make_shared<Mesh>("Double Quad", FixPath("../../Assets/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/sphere.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("Torus", FixPath("../../Assets/torus.obj").c_str());

	// Create the floor to test shadows
	std::shared_ptr<GameEntity> ground = std::make_shared<GameEntity>(cubeMesh, woodMat);
	ground->GetTransform()->SetScale(25, 25, 25);
	ground->GetTransform()->SetPosition(0, -28, 15);
	entities.push_back(ground);

	// Create the examples for the other shaders
	for (int i = 1; i < 20; i++)
	{
		// Create GameEntity with the bronze material and the cube mesh
		std::shared_ptr<Mesh> mesh = cubeMesh;
		std::shared_ptr<GameEntity> fogEntity = std::make_shared<GameEntity>(mesh, bronzeMat);

		// Set up its transform to be the same but offset in the z and scale it to be taller
		fogEntity->GetTransform()->Scale(1.0f, 3.0f, 1.0f);
		fogEntity->GetTransform()->MoveAbsolute(3.0f, 0, 5.0f * i);

		// Put the new entity in the list
		entities.push_back(fogEntity);
	}

	// Create the examples for the other shaders
	for (int i = 1; i < 20; i++)
	{
		// Create GameEntity with the bronze material and the cube mesh
		std::shared_ptr<Mesh> mesh = cubeMesh;
		std::shared_ptr<GameEntity> fogEntity = std::make_shared<GameEntity>(mesh, bronzeMat);

		// Set up its transform to be the same but offset in the z and scale it to be taller
		fogEntity->GetTransform()->Scale(1.0f, 3.0f, 1.0f);
		fogEntity->GetTransform()->MoveAbsolute(-3.0f, 0, 5.0f * i);

		// Put the new entity in the list
		entities.push_back(fogEntity);
	}

	// Create our Ambient Color
	ambientColor = XMFLOAT4(0, 0, 0, 1.0f);

	// Create Lights
	Light dLight1 = {};
	dLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	dLight1.Direction = XMFLOAT3(1, -1, 1);
	dLight1.Color = XMFLOAT3(1, 1, 1);
	dLight1.Intensity = 1.0f;

	Light dLight2 = {};
	dLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dLight2.Direction = XMFLOAT3(0, -1, 1);
	dLight2.Color = XMFLOAT3(1, 1, 1);
	dLight2.Intensity = 1.0f;

	Light dLight3 = {};
	dLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dLight3.Direction = XMFLOAT3(0, -1, -1);
	dLight3.Color = XMFLOAT3(1, 1, 1);
	dLight3.Intensity = 1.0f;

	Light pointLight1 = {};
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Position = XMFLOAT3(1, 0, 0);
	pointLight1.Color = XMFLOAT3(1, 0, 0);
	pointLight1.Intensity = 10.0f;
	pointLight1.Range = 1.0f;

	Light spotLight = {};
	spotLight.Type = LIGHT_TYPE_SPOT;
	spotLight.Position = XMFLOAT3(-1, -1, 0);
	spotLight.Direction = XMFLOAT3(0, -1, 0);
	spotLight.Color = XMFLOAT3(0, 0, 1);
	spotLight.Intensity = 0.5f;
	spotLight.Range = 10.0f;
	spotLight.SpotInnerAngle = XMConvertToRadians(20.0f);
	spotLight.SpotOuterAngle = XMConvertToRadians(45.0f);

	lights.push_back(dLight1);
	lights.push_back(dLight2);
	lights.push_back(dLight3);
	//lights.push_back(pointLight1);
	//lights.push_back(spotLight);

	// Create our light view matrix
	XMVECTOR lightDirection = XMLoadFloat3(&dLight1.Direction);		// Convert our light.direction from a float3 to a xmvector
	XMMATRIX lightView = XMMatrixLookToLH(
		-lightDirection * 20, // Position: "Backing up" 20 units from origin
		lightDirection, // Direction: light's direction
		XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	// Create our light projection matrix
	float lightProjectionSize = 30.0f; // Tweak for your scene!
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);

	// Create our skybox
	sky = std::make_shared<Sky>(
		samplerState,
		cubeMesh,
		skyPixelShader,
		skyVertexShader,
		FixPath(L"../../Assets/Planet/right.png").c_str(),
		FixPath(L"../../Assets/Planet/left.png").c_str(),
		FixPath(L"../../Assets/Planet/up.png").c_str(),
		FixPath(L"../../Assets/Planet/down.png").c_str(),
		FixPath(L"../../Assets/Planet/front.png").c_str(),
		FixPath(L"../../Assets/Planet/back.png").c_str()
		);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Check if the cameras exists yet
	for (int i = 0; i < cameras.size(); i++)
	{
		if (cameras[i] != NULL)
		{
			// Update the projection matrix with the new aspect ratio
			cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}
}

// --------------------------------------------------------
// Variables. I know they shouldn't go here but I need to
// keep track of them and for now this is fine
// --------------------------------------------------------
int number;
XMFLOAT4 color( 0.4f, 0.6f, 0.75f, 0.0f );
bool demoWindow = true;

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Put this all in a helper method that is called from Game::Update()
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);

	// --------------------------------------------------------------------------------------

	// holder of which camera is active right now
	std::shared_ptr<Camera> currentCam;

	// Bool for showing the demo window
	bool demoWindow = true;



	// Begin Custom Window
	ImGui::Begin("Inspector");
	{

		// Put the App details in a tree so they aren't always visible
		if (ImGui::TreeNode("App Details"))
		{
			// Framerate and resolution
			ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
			ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());
			
			// Button to turn the demo window off
			if (ImGui::Button("Show ImGui Demo Window"))
			{
				demoWindow = !demoWindow;
			}

			// 4 Part float editor to change our background color
			ImGui::DragFloat4("Background Color", &color[0]);
			
			// End the tree
			ImGui::TreePop();
		}

		// Tree to show mesh details
		if (ImGui::TreeNode("Mesh Details"))
		{
			for (int m = 0; m < meshes.size(); m++)
			{
				// So that the ID names don't conflict if there are meshes of the same name
				ImGui::PushID(meshes[m].get());

				// Child node for each mesh
				if (ImGui::TreeNode("Mesh", "Mesh %s", meshes[m]->GetName()))
				{
					ImGui::Text("Triangles: %d", meshes[m]->GetIndexCount() / 3);
					ImGui::Text("Vertices: %d", meshes[m]->GetVertexCount());
					ImGui::Text("Indices: %d", meshes[m]->GetIndexCount());
					// Pop the tree for the next child node
					ImGui::TreePop();
				}
				// Pop the id for a new one
				ImGui::PopID();
			}
			// End the Meshes tree
			ImGui::TreePop();
		}

		// Tree to show entity details
		if (ImGui::TreeNode("Entity details"))
		{
			for (int e = 0; e < entities.size(); e++)
			{
				ImGui::PushID(entities[e].get());

				// Child node for each entity
				if (ImGui::TreeNode("Entity", "Entity &d", e))
				{
					// Mesh info
					ImGui::Text("Mesh: %s", entities[e]->GetMesh()->GetName());

					// Transform info
					std::shared_ptr<Transform> transformHolder = entities[e]->GetTransform();
					XMFLOAT3 position = transformHolder->GetPosition();
					XMFLOAT3 rotation = transformHolder->GetPitchYawRoll();
					XMFLOAT3 scale = transformHolder->GetScale();

					// Reset the transform values based on the drag float widgets
					if (ImGui::DragFloat3("Position", &position.x, 0.1f))
					{
						transformHolder->SetPosition(position);
					}
					if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f))
					{
						transformHolder->SetRotation(rotation);
					}
					if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
					{
						transformHolder->SetScale(scale);
					}
					// End Child node
					ImGui::TreePop();
				}
				// Pop ID to renew
				ImGui::PopID();
			}
			// End Entity Tree
			ImGui::TreePop();
		}

		// Camera Details
		if (ImGui::TreeNode("Camera Details"))
		{
			// Buttons to switch active cameras (this only works because I only made 2 cameras)
			if (ImGui::Button("Main Camera"))
			{
				cameras[0]->SetActive(true);
				cameras[1]->SetActive(false);
			}
			if (ImGui::Button("Alternate Camera"))
			{
				cameras[0]->SetActive(false);
				cameras[1]->SetActive(true);
			}

			// Loop to see which camera is active, if two are active, the latter in the list will be chosen
			for (int i = 0; i < cameras.size(); i++)
			{
				if (cameras[i]->GetActive()) { currentCam = cameras[i]; }
			}

			ImGui::Spacing();

			// Text for the camera's position
			ImGui::Text("Camera Position:");
			std::shared_ptr<Transform> transform = currentCam->GetTransform();
			ImGui::BulletText("X: %f", transform->GetPosition().x);
			ImGui::BulletText("Y: %f", transform->GetPosition().y);
			ImGui::BulletText("Z: %f", transform->GetPosition().z);

			ImGui::Spacing();

			float moveSpeed = currentCam->GetMoveSpeed();
			// Drag float to change camera movement speed
			if (ImGui::DragFloat("Movement Speed", &moveSpeed, 0.1f))
			{
				currentCam->SetMoveSpeed(moveSpeed);
			}

			float sensitivity = currentCam->GetSensativity();	// I now realize I mispelled sensitivity
			// Drag float to change camera sensitivity
			if (ImGui::DragFloat("Sensitivity", &sensitivity, 0.0001f))
			{
				currentCam->SetSensativity(sensitivity);
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Materials"))
		{
			for (int m = 0; m < materials.size(); m++)
			{
				ImGui::PushID(materials[m].get());

				if (ImGui::TreeNode("Material", "Material %s", materials[m]->GetName()))
				{
					XMFLOAT2 scale = materials[m]->GetuvScale();
					if (ImGui::DragFloat2("UV Scale: ", &scale.x, 0.5f))
					{
						materials[m]->SetuvScale(scale);
					}

					ImGui::Spacing();

					XMFLOAT2 offset = materials[m]->GetuvOffset();
					if (ImGui::DragFloat2("UV Offset: ", &offset.x, 0.5f))
					{
						materials[m]->SetuvOffset(offset);
					}

					ImGui::Spacing();

					XMFLOAT4 tint = materials[m]->GetColorTint();
					if (ImGui::ColorEdit4("Color Tint", &tint.x))
					{
						materials[m]->SetColorTint(tint);
					}

					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			ImGui::TreePop(); 
		}
		if (ImGui::TreeNode("Lights"))
		{
			// Editor for the ambient color
			ImGui::Spacing();
			ImGui::ColorEdit4("Ambient Light Color", &ambientColor.x);

			// Loop to go through each light
			for (int l = 0; l < lights.size(); l++)
			{
				// Name of the light
				std::string name;

				if (lights[l].Type == LIGHT_TYPE_DIRECTIONAL)
				{
					name = "Directional";
				}
				else if (lights[l].Type == LIGHT_TYPE_SPOT)
				{
					name = "Spot";
				}
				else if (lights[l].Type == LIGHT_TYPE_POINT)
				{
					name = "Point";
				}

				name += " Light %d";

				// Editing for each individual light
				ImGui::PushID(l);
				if (ImGui::TreeNode("Light Node", name.c_str(), l))
				{
					// Build UI for one entity at a time
					ImGui::ColorEdit3("Color", &lights[l].Color.x);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Post Processing"))
		{
			// Drag float for blur radius
			ImGui::DragInt("Blur Radius", &blurRadius, 1);

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Fog"))
		{
			// Fog type
			if (ImGui::RadioButton("Exponential", 0))
			{
				fogType = 0;
			}
			ImGui::SameLine();

			if (ImGui::RadioButton("Parameterized", 1))
			{
				fogType = 1;
			}
			ImGui::SameLine();

			if (ImGui::RadioButton("Linear", 2))
			{
				fogType = 2;
			}

			// Exponential
			if (fogType == 0)
			{
				ImGui::DragFloat("Density", &fogDensity, 0.001f);
			}

			// Parameterized
			if (fogType == 1)
			{
				ImGui::DragFloat("Fog Start", &startFog, 1.0f);
				ImGui::DragFloat("Full Fog", &fullFog, 1.0f);
			}

			// Parameterized
			if (fogType == 2)
			{
				//ImGui::Text("Camera Far Clip Plane: ", currentCam->GetFarClip());
			}

			ImGui::TreePop();
		}
	}
	ImGui::End(); // Ends the current window

	if (demoWindow)
	{
		// Show the demo window
		ImGui::ShowDemoWindow();
	}

	// Loop to see which camera is active, if two are active, the latter in the list will be chosen
	for (int i = 0; i < cameras.size(); i++)
	{
		if (cameras[i]->GetActive()) { currentCam = cameras[i]; }
	}

	// Update the camera
	currentCam->Update(deltaTime);

	// --------------------------------------------------------------------------------------

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Move some entities every frame
	//entities[3]->GetTransform()->Rotate(0, 0, deltaTime * 1.0f);
	//entities[0]->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
	//entities[1]->GetTransform()->SetPosition(0, deltaTime * 1.0f, 0);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	&color[0]);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	

	//-------------------------------------------------------------
	// Shadow Mapping
	//-------------------------------------------------------------
	// Clear the shadow map
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// set up output merger state
	ID3D11RenderTargetView* nullRTV{};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Enable the rasterizer state
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	// Deactivate pixel shader
	Graphics::Context->PSSetShader(0, 0, 0);

	// Change Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	// holder of which camera is active right now
	std::shared_ptr<Camera> currentCam;

	// Loop to see which camera is active, if two are active, the latter in the list will be chosen
	for (int i = 0; i < cameras.size(); i++)
	{
		if (cameras[i]->GetActive()) { currentCam = cameras[i]; }
	}

	// Entity render loop
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", lightViewMatrix);
	shadowVS->SetMatrix4x4("projection", lightProjectionMatrix);

	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e->GetMesh()->Draw();
	}

	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

	// Diasable the rasterizer state
	Graphics::Context->RSSetState(0);

	// Post Processing Pre-render
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Graphics::Context->ClearRenderTargetView(ppRTV.Get(), clearColor);
	Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

	// Draw the geometry
	// Loop through game entities list to draw each
	for (auto& e : entities)
	{
		// Lighting and shader stuff
		e->GetMaterial()->GetPixelShader()->SetFloat4("ambient", ambientColor);

		std::shared_ptr<SimplePixelShader> ps = e->GetMaterial()->GetPixelShader();
		std::shared_ptr<SimpleVertexShader> vs = e->GetMaterial()->GetVertexShader();

		vs->SetMatrix4x4("lightView", lightViewMatrix);
		vs->SetMatrix4x4("lightProjection", lightProjectionMatrix);

		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		ps->SetInt("lightCount", (int)lights.size());

		ps->SetShaderResourceView("ShadowMap", shadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);

		// Fog business
		ps->SetInt("fogType", fogType);
		ps->SetFloat("farClip", currentCam->GetFarClip());
		ps->SetFloat3("fogColor", fogColor);
		ps->SetFloat("startFog", startFog);
		ps->SetFloat("fullFog", fullFog);
		ps->SetFloat("fogDensity", fogDensity);

		// Draw the entity
		e->Draw(currentCam);
	}

	// Draw the skybox after everything else
	sky->Draw(currentCam);

	// -----------------------------------------------------------------------------------
	// Post Processing Post-draw
	// -----------------------------------------------------------------------------------
	// restore the back buffer
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

	// Turn off vertex and index buffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* nothing = 0;
	Graphics::Context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
	Graphics::Context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);

	// Activate shaders
	fullscreenVS->SetShader();
	boxBlurPS->SetShader();

	// cBuffer resources
	boxBlurPS->SetFloat("pixelWidth", 1.0f / Window::Width());
	boxBlurPS->SetFloat("pixelHeight", 1.0f / Window::Height());
	boxBlurPS->SetInt("blurRadius", blurRadius);

	// Binding
	boxBlurPS->SetShaderResourceView("Pixels", ppSRV.Get());
	boxBlurPS->SetSamplerState("ClampSampler", ppSampler.Get());

	// Copy data
	boxBlurPS->CopyAllBufferData();

	Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

	// Unbind the shadow map at end of frame
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Draw the UI after everything else
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}



