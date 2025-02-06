#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// Structure to help the COnstant Buffer match between
// CPU and GPU space
// --------------------------------------------------------
struct BufferStruct {

	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT3 offset;
};