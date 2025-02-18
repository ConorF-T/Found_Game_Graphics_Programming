#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include "Graphics.h"
#include "Vertex.h"

class Mesh
{
public:
	// Basic OOP Setup
	Mesh(Vertex vertices[], int verticesSize, unsigned int indices[], int indicesSize);
	~Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator = (const Mesh&) = delete;

	// Public Methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	int GetVertexCount();
	void Draw();

private:
	// Buffers for geometric data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Integers for keeping track of vertext and index buffer numbers
	int numVertices;
	int numIndices;

};



