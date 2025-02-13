#include "GameEntity.h"
#include "BufferStruct.h"

#include "Graphics.h"

using namespace DirectX;

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh) : mesh(mesh)
{
	transform = std::make_shared<Transform>();
}

GameEntity::~GameEntity()
{
}


// Setter
void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }


// Getters
std::shared_ptr<Mesh> GameEntity::GetMesh() { return mesh; }
std::shared_ptr<Transform> GameEntity::GetTransform() { return transform; }


// Draw Method
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer)
{
	// Constant Buffer Business
	BufferStruct vsData;
	vsData.colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.world = transform->GetWorldMatrix();

	// Mapping and unmapping the buffer
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);

	// Draw the mesh using m the mesh draw function
	mesh->Draw();
}
