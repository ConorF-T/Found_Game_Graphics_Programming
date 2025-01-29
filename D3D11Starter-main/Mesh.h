#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class Mesh
{
public:
	// Basic OOP Setup
	Mesh() = default;
	~Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator = (const Mesh&) = delete;

	// Public Methods
	void GetVertexBuffer();
	void GetIndexBuffer();
	int GetIndexCount();
	int GetVertexCount();
	void Draw(float deltaTime, float totalTime);

private:
	// Buffers for geometric data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Integers for keeping track of vertext and index buffer numbers
	int indexBufferIndices;
	int vertexBufferVertices;

};



