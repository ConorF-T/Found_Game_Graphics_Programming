#include "Camera.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera( DirectX::XMFLOAT3 position, float fieldOfView, float aspectRatio, float nearClipPlane, float farClipPlane, bool perspective, float movementSpeed, float mouseLookSpeed) :
	fieldOfView(fieldOfView),
	aspectRatio(aspectRatio),
	nearClipPlane(nearClipPlane),
	farClipPlane(farClipPlane),
	perspective(perspective),
	movementSpeed(movementSpeed),
	mouseLookSpeed(mouseLookSpeed)
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
	// Create a speed to use to update transform
	float dtSpeed = movementSpeed * dt;

	// WASD movement (relative)
	if (Input::KeyDown('W')) { transform->MoveRelative(0, 0, dtSpeed); }	// Forward into screen
	if (Input::KeyDown('A')) { transform->MoveRelative(-dtSpeed, 0, 0); }	// Left
	if (Input::KeyDown('S')) { transform->MoveRelative(0, 0, -dtSpeed); }	// Back towards person
	if (Input::KeyDown('D')) { transform->MoveRelative(dtSpeed, 0, 0); }	// Right

	// Up/down movement (absolute)
	if (Input::KeyDown('Q')) { transform->MoveAbsolute(0, dtSpeed, 0); }	// Up
	if (Input::KeyDown('E')) { transform->MoveAbsolute(0, -dtSpeed, 0); }	// Down

	// Mouse looking around
	if (Input::MouseRightDown())
	{
		// Get how much the mouse has moved in either X or Y this frame
		int cursorMovementX = Input::GetMouseXDelta() * mouseLookSpeed;
		int cursorMovementY = Input::GetMouseYDelta() * mouseLookSpeed;
		
		// Rotate the camera
		transform->Rotate(cursorMovementY, cursorMovementX, 0);	// Flipped due to how pitch and yaw work

		// Clamp so that we don't go upside down
		XMFLOAT3 rotationHolder = transform->GetPitchYawRoll();
		if (rotationHolder.x > XM_PIDIV2) { rotationHolder.x = XM_PIDIV2; }
		if (rotationHolder.x < -XM_PIDIV2) { rotationHolder.x = -XM_PIDIV2; }
	}

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

bool Camera::GetActive()
{
	return isActive;
}

float Camera::GetMoveSpeed()
{
	return 0.0f;
}

float Camera::GetSensativity()
{
	return 0.0f;
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

void Camera::SetActive(bool active)
{
	isActive = active;
}

void Camera::SetMoveSpeed(float speed)
{
	movementSpeed = speed;
}

void Camera::SetSensativity(float speed)
{
	mouseLookSpeed = speed;
}
