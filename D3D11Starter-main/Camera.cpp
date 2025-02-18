#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera( DirectX::XMFLOAT3 position, float fieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane, bool perspective) :
	fieldOfView(fieldOfView),
	aspectRatio(aspectRatio),
	nearClipPlane(nearClipPlane),
	farClipPlane(farClipPlane),
	perspective(perspective)
{
	// Make shared ptr for transform
	transform = std::make_shared<Transform>();
	// Set position to that transform shared ptr
	transform->SetPosition(position);

	// Update view and projection matrices
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	this->aspectRatio = aspectRatio;

	XMMATRIX projection;

	// Different methods determined by perspective or orthographic
	if (perspective)	// Perspective
	{
		projection = XMMatrixPerspectiveFovLH(
			fieldOfView,		
			aspectRatio,		
			nearClipPlane,			
			farClipPlane);			
	}
	else	// Orthographic
	{
		projection = XMMatrixOrthographicLH(
			orthographicWidth,
			orthographicWidth / aspectRatio,
			nearClipPlane, 
			farClipPlane);
	}

	// Store time
	XMStoreFloat4x4(&projectionMatrix, projection);
}

void Camera::UpdateViewMatrix()
{
	// Get our forward and position to use later
	XMFLOAT3 forward = transform->GetForward();
	XMFLOAT3 position = transform->GetPosition();

	// Make a new view matrix
	XMMATRIX newMatrix = XMMatrixLookToLH(
		XMLoadFloat3(&position),
		XMLoadFloat3(&forward),
		XMVectorSet(0, 1, 0, 0));

	XMStoreFloat4x4(&viewMatrix, newMatrix);
}

void Camera::Update(float dt)
{
	// Update the view matrix every frame
	UpdateViewMatrix();
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projectionMatrix;
}

std::shared_ptr<Transform> Camera::GetTransform()
{
	return transform;
}

float Camera::GetAspectRatio()
{
	return aspectRatio;
}

float Camera::GetFieldOfView()
{
	return fieldOfView;
}

float Camera::GetNearClip()
{
	return nearClipPlane;
}

float Camera::GetFarClip()
{
	return farClipPlane;
}

float Camera::GetOrthographicWidth()
{
	return orthographicWidth;
}

bool Camera::GetProjectionType()
{
	return perspective;
}

void Camera::SetFieldOfView(float fov)
{
	fieldOfView = fov;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetNearClip(float distance)
{
	nearClipPlane = distance;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetFarClip(float distance)
{
	farClipPlane = distance;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetOrthographicWidth(float width)
{
	orthographicWidth = width;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetProjectionType(bool perspectiveBool)
{
	perspective = perspectiveBool;
	UpdateProjectionMatrix(aspectRatio);
}
