#include "Material.h"

// Constructor
Material::Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader) :
	colorTint(colorTint),
	vertexShader(vertexShader),
	pixelShader(pixelShader)

{

}


// Getters
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vertexShader; }

std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return pixelShader; }


// Setters
void Material::SetColorTint(DirectX::XMFLOAT4 color) { this->colorTint = color; }

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vs) { this->vertexShader = vs; }

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> ps) { this->pixelShader = ps; }
