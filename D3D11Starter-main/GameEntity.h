#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "Transform.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh);
	~GameEntity();

	// Setter
	void SetMesh(std::shared_ptr<Mesh> mesh);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	// Draw Method
	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer);

private:
	// Mehs and transform
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
};

