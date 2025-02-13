#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "Transform.h"
#include <memory>
#include "BufferStruct.h"

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

// Meshes
std::shared_ptr<Mesh> mainTriangle;
std::shared_ptr<Mesh> rectangle;
std::shared_ptr<Mesh> polygon;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
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

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
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

	// Constant Buffer
	
	// Calculate the next multiple of 16 (instead of hardcoding it)
	unsigned int size = sizeof(BufferStruct);
	size = (size + 15) / 16 * 16;

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // 
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	Graphics::Device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

	// Bind the Constant Buffer to rendering pipeline
	Graphics::Context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		vsConstantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
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

	// ----- Triangle Mesh -----
	Vertex triVerts[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green }
	};
	unsigned int triIndices[] = { 0, 1, 2 };

	
	// ----- Rectangle Mesh -----
	Vertex rectVerts[] =
	{
		{ XMFLOAT3(-0.7f, +0.7f, +0.0f), red },
		{ XMFLOAT3(-0.3f, +0.7f, +0.0f), blue },
		{ XMFLOAT3(-0.3f, +0.3f, +0.0f), blue },
		{ XMFLOAT3(-0.7f, +0.3f, +0.0f), red },
	};
	unsigned int rectIndices[] = 
	{ 
		0, 1, 2, 
		0, 2, 3 
	};

	// ----- Polygon Mesh -----
	Vertex polyVerts[] =
	{
		{ XMFLOAT3(+0.3f, +0.5f, +0.0f), white },
		{ XMFLOAT3(+0.4f, +0.7f, +0.0f), black },
		{ XMFLOAT3(+0.6f, +0.6f, +0.0f), black },
		{ XMFLOAT3(+0.5f, +0.5f, +0.0f), white },
		{ XMFLOAT3(+0.6f, +0.4f, +0.0f), black },
		{ XMFLOAT3(+0.4f, +0.3f, +0.0f), black },
	};
	unsigned int polyIndices[] = 
	{ 
		0, 1, 2, 
		2, 3, 0, 
		3, 4, 5, 
		5, 0, 3 
	};

	// Create the base meshes with the vertex and index buffers
	std::shared_ptr<Mesh> tri = std::make_shared<Mesh>("Triangle", triVerts, ARRAYSIZE(triVerts), triIndices, ARRAYSIZE(triIndices));
	std::shared_ptr<Mesh> rect = std::make_shared<Mesh>("Rectangle", rectVerts, ARRAYSIZE(rectVerts), rectIndices, ARRAYSIZE(rectIndices));
	std::shared_ptr<Mesh> poly = std::make_shared<Mesh>("Polygon", polyVerts, ARRAYSIZE(polyVerts), polyIndices, ARRAYSIZE(polyIndices));

	meshes.push_back(tri);
	meshes.push_back(rect);
	meshes.push_back(poly);

	// Create the game entities
	std::shared_ptr<GameEntity> g1 = std::make_shared<GameEntity>(tri);
	std::shared_ptr<GameEntity> g2 = std::make_shared<GameEntity>(rect);
	std::shared_ptr<GameEntity> g3 = std::make_shared<GameEntity>(poly);
	std::shared_ptr<GameEntity> g4 = std::make_shared<GameEntity>(tri);
	std::shared_ptr<GameEntity> g5 = std::make_shared<GameEntity>(rect);

	// Add to entity vector (so Chris doesn't have to see copy-paste code)
	entities.push_back(g1);
	entities.push_back(g2);
	entities.push_back(g3);
	entities.push_back(g4);
	entities.push_back(g5);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
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
	

	// Begin Custom Window
	ImGui::Begin("Inspector");
	{
		// Label width
		ImGui::PushItemWidth(-160);

		if (ImGui::TreeNode("App Details"))
		{
			ImGui::Spacing();
			// Framerate
			ImGui::Text("Frame rate: %f fps", ImGui::GetIO().Framerate);
			// Window size
			ImGui::Text("Window Client Size: %dx%d", Window::Width(), Window::Height());

			// Demo window button
			if (ImGui::Button(demoWindow ? "Hide ImGui Demo Window" : "Show ImGui Demo Window"))
			{
				demoWindow = !demoWindow;
			}

			ImGui::Spacing();

			ImGui::TreePop();
		}

		// Meshes
		if (ImGui::TreeNode("Meshes"))
		{
			// Loop through each mesh to show their details in the tree
			for (int i = 0; i < meshes.size(); i++)
			{
				ImGui::PushID(meshes[i].get());

				if (ImGui::TreeNode("Mesh Node", "Mesh: %s", meshes[i]->GetName()))
				{
					ImGui::Spacing();
					ImGui::Text("Triangles: %d", meshes[i]->GetIndexCount() / 3);
					ImGui::Text("Vertices:  %d", meshes[i]->GetVertexCount());
					ImGui::Text("Indices:   %d", meshes[i]->GetIndexCount());
					ImGui::Spacing();
					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		/*
		// Entities
		if (ImGui::TreeNode("Entities"))
		{
			// Loop through each mesh to show their details in the tree
			for (int i = 0; i < entities.size(); i++)
			{
				ImGui::PushID(meshes[i].get());

				if (ImGui::TreeNode("Entity Node", "Entity: %s", i))
				{
					// Mesh info
					ImGui::Spacing();
					ImGui::Text("Mesh: %s", entities[i]->GetMesh()->GetName());
					ImGui::Spacing();

					// Transform info
					std::shared_ptr<Transform> transform = entities[i]->GetTransform();
					XMFLOAT3 position = transform->GetPosition();
					XMFLOAT3 rotation = transform->GetPitchYawRoll();
					XMFLOAT3 scale = transform->GetScale();

					// Position rotation and scale
					if (ImGui::DragFloat3("Position", &position.x, 0.01f)) transform->SetPosition(position);
					if (ImGui::DragFloat3("Rotation (Radians)", &rotation.x, 0.01f)) transform->SetRotation(rotation);
					if (ImGui::DragFloat3("Scale", &scale.x, 0.01f)) transform->SetScale(scale);

					ImGui::Spacing();

					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			ImGui::TreePop();
		}
		*/
	}
	ImGui::End(); // Ends the current window

	if (demoWindow)
	{
		// Show the demo window
		ImGui::ShowDemoWindow();
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// Draw the geomtry
	// Loop through game entities list to draw each
	for (auto& e : entities)
	{
		e->Draw(vsConstantBuffer);
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



