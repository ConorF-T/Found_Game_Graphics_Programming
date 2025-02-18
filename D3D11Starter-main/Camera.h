#pragma once
#include <DirectXMath.h>

#include "Transform.h"
#include <memory>

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 position, float fieldOfView, float aspectRatio, float nearClip, float farClip, bool perspective);
	~Camera();

	// Updating methods
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);

	// Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	std::shared_ptr<Transform> GetTransform();

	float GetAspectRatio();
	float GetFieldOfView();
	float GetNearClip();
	float GetFarClip();
	float GetOrthographicWidth();
	bool GetProjectionType();

	// Setters
	void SetFieldOfView(float fov);
	void SetNearClip(float distance);
	void SetFarClip(float distance);
	void SetOrthographicWidth(float width);
	void SetProjectionType(bool perspectiveBool);

private:
	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Camera Transform
	std::shared_ptr<Transform> transform;

	// Semi-optional customization
	float fieldOfView;
	float aspectRatio;
	float nearClipPlane;
	float farClipPlane;
	float orthographicWidth;

	// Whether the camera is in perspective or orthographic mode
	bool perspective;

};

