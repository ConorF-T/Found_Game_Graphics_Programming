#pragma once

#include "Transform.h"
#include <memory>

#include <DirectXMath.h>

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 pos, float moveSpeed, float lookSpeed, float fov,  float aspectRatio);
	~Camera();

	// Update Methods
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdteProjectMatrix(float aspectRatio);

	// Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();

private:
	// Camera Matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	// Transform
	//std::shared_ptr<Transform> transform;

	// Other Camera related stuff
	float fieldOfView;
	float movementSpeed;
};

