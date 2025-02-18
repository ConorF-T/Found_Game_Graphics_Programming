#include "Camera.h"

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 pos, float moveSpeed, float lookSpeed, float fov, float aspectRatio)
{

}

Camera::~Camera()
{

}

void Camera::Update(float dt)
{
	float speed = dt * movementSpeed;

	if (Input::KeyDown('W')) { transform->Moverelative(0, 0, speed); }
	if (Input::KeyDown('A')) { transform->Moverelative(-0, 0, -speed); }
	if (Input::KeyDown('S')) { transform->Moverelative(-speed, 0, 0); }
	if (Input::KeyDown('D')) { transform->Moverelative(speed, 0, 0); }
	if (Input::KeyDown(' ')) { transform->Moverelative(0, speed, 0); }
	if (Input::KeyDown('X')) { transform->Moverelative(0, -speed, 0); }

	// Only rotate when clicking mouse
	if (Input::MouseLeftDown())
	{
		// how much did the cursor move?
		float yRot = mouseLookSpeed * Input::GetMouseXDelta();
		float xRot = mouseLookSpeed * Input::GetMouseYDelta();

		transform->Rotate(yRot, xRot, 0);
	}

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform->GetPosition();
	XMFLOAT3 fwd = transform->GetForward();
	XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);


	XMMatrixLookAtLH(
		XMLoadFloat3(&pos0),
		XMLoadFloat3(&fwd),
		XMLoadFloat3(&worldUp));
}

void Camera::UpdteProjectMatrix(float aspectRatio)
{
	XMMatrixPerspectiveFovLH(
		XM_PIDIV4,
		aspectRatio,
		0.01f,
		100.0f);
}

DirectX::XMFLOAT4X4 Camera::GetView() {  return viewMatrix;  }
DirectX::XMFLOAT4X4 Camera::GetProjection() { return projMatrix; }
