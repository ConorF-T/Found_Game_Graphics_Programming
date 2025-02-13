#include "Transform.h"

using namespace DirectX;

Transform::Transform() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1)
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

Transform::~Transform()
{
}
