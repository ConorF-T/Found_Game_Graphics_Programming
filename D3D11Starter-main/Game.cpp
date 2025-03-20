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
		XMFLOAT3(0.0f, 0.0f, -5.0f),
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
	std::shared_ptr<SimpleVertexShader> vertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> uvPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> customPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> DecalPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DecalPixelShader.cso").c_str());

	// Load some textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/brick_texture.jpg").c_str(), nullptr, brickSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sandSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/sand_texture.jpg").c_str(), nullptr, sandSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dirtSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/dirt_texture.png").c_str(), nullptr, dirtSRV.GetAddressOf());

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
	std::shared_ptr<Material> matBricks = std::make_shared<Material>(white, vertexShader, pixelShader);
	matBricks->AddSampler("BasicSampler", samplerState);
	matBricks->AddTextureSRV("SurfaceTexture", brickSRV);

	std::shared_ptr<Material> matSand = std::make_shared<Material>(white, vertexShader, pixelShader);
	matSand->AddSampler("BasicSampler", samplerState);
	matSand->AddTextureSRV("SurfaceTexture", sandSRV);

	std::shared_ptr<Material> matDirtBricks = std::make_shared<Material>(white, vertexShader, DecalPixelShader);
	matDirtBricks->AddSampler("BasicSampler", samplerState);
	matDirtBricks->AddTextureSRV("SurfaceTexture", brickSRV);
	matDirtBricks->AddTextureSRV("DecalTexture", dirtSRV);

	// Add materials to vector
	materials.insert(materials.end(), { matBricks, matSand, matDirtBricks });

	// Create our meshes with .obj files
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("Cube", FixPath("../../Assets/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("Helix", FixPath("../../Assets/helix.obj").c_str());
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>("Quad", FixPath("../../Assets/quad.obj").c_str());
	std::shared_ptr<Mesh> dubQuadMesh = std::make_shared<Mesh>("Double Quad", FixPath("../../Assets/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>("Sphere", FixPath("../../Assets/sphere.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("Torus", FixPath("../../Assets/torus.obj").c_str());

	// Create the GameEntities
	std::shared_ptr<GameEntity> gameCube = std::make_shared<GameEntity>(cubeMesh, matDirtBricks);
	std::shared_ptr<GameEntity> gameCylinder = std::make_shared<GameEntity>(cylinderMesh, matDirtBricks);
	std::shared_ptr<GameEntity> gameHelix = std::make_shared<GameEntity>(helixMesh, matBricks);
	std::shared_ptr<GameEntity> gameQuad = std::make_shared<GameEntity>(quadMesh, matBricks);
	std::shared_ptr<GameEntity> gameDubQuad = std::make_shared<GameEntity>(dubQuadMesh, matBricks);
	std::shared_ptr<GameEntity> gameSphere = std::make_shared<GameEntity>(sphereMesh, matBricks);
	std::shared_ptr<GameEntity> gameTorus = std::make_shared<GameEntity>(torusMesh, matBricks);

	// Add the entities to the entities list
	entities.push_back(gameCube);
	entities.push_back(gameCylinder);
	entities.push_back(gameHelix);
	entities.push_back(gameQuad);
	entities.push_back(gameDubQuad);
	entities.push_back(gameSphere);
	entities.push_back(gameTorus);

	// Adjust the transforms
	float adjust = -9.0f;
	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->MoveAbsolute(adjust, 0.0f, 10.0f);
		adjust += 3.0f;
	}

	// Adjusting the quads and torus to see them better
	entities[3]->GetTransform()->Rotate(-1.0f, 0, 0);
	entities[4]->GetTransform()->Rotate(1.0f, 0, 0);
	entities[6]->GetTransform()->Rotate(1.5f, 0, 0);

	// Create the examples for the other shaders
	for (int i = 0; i < 7; i++)
	{
		// Create GameEntity with the UV material and the list item's mesh
		std::shared_ptr<Mesh> mesh = entities[i]->GetMesh();
		std::shared_ptr<GameEntity> gameSand = std::make_shared<GameEntity>(mesh, matSand);

		// Set up its transform to be the same but offset in the y
		gameSand->GetTransform()->MoveAbsolute(entities[i]->GetTransform()->GetPosition());
		gameSand->GetTransform()->MoveAbsolute(0, -3.0f, 0);

		// Put both the new entities in the list
		entities.push_back(gameSand);
	}
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

				if (ImGui::TreeNode("Material", "Material %d", m))
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

	// holder of which camera is active right now
	std::shared_ptr<Camera> currentCam;

	// Loop to see which camera is active, if two are active, the latter in the list will be chosen
	for (int i = 0; i < cameras.size(); i++)
	{
		if (cameras[i]->GetActive()) { currentCam = cameras[i]; }
	}

	// Draw the geometry
	// Loop through game entities list to draw each
	for (auto& e : entities)
	{
		e->Draw(currentCam);
	}

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



