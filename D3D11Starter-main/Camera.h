#pragma once
#include <DirectXMath.h>

#include "Transform.h"
#include <memory>

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 position, float fieldOfView, float aspectRatio, float nearClip, float farClip, bool perspective, float movementSpeed, float mouseLookSpeed);
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
	bool GetActive();
	float GetMoveSpeed();
	float GetSensativity();

	// Setters
	void SetFieldOfView(float fov);
	void SetNearClip(float distance);
	void SetFarClip(float distance);
	void SetOrthographicWidth(float width);
	void SetProjectionType(bool perspectiveBool);
	void SetActive(bool active);
	void SetMoveSpeed(float speed);
	void SetSensativity(float speed);

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
	float movementSpeed;
	float mouseLookSpeed;

	// Whether the camera is in perspective or orthographic mode
	bool perspective;

	// Whether the camera is active or not
	bool isActive;

};