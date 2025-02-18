#include "Mesh.h"
#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

#include "ImGui/imgui_impl_win32.h"

using namespace DirectX;

Mesh::Mesh (Vertex vertices[], int verticesSize, unsigned int indices[], int indicesSize)
{
	
	numIndices = indicesSize;
	numVertices = verticesSize;

	// Create the vertex buffer using our passed vertices
	{
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * numVertices; // Number of vertices related to input
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices;

		Graphics::Device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	// Create the index buffer using our passed indices
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(unsigned int) * numIndices; // Number of indices related to input
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		// Specify the initial data for this buffer, similar to above
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	}
}


// --------------------------------------------------------
// Destructor to clean up memory objects
// --------------------------------------------------------
Mesh::~Mesh()
{

}


Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

int Mesh::GetVertexCount()
{
	return numVertices;
}

int Mesh::GetIndexCount()
{
	return numIndices;
}

void Mesh::Draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, this->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(this->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	Graphics::Context->DrawIndexed(
		this->GetIndexCount(),
		0,
		0);
}