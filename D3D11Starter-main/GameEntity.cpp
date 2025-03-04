#include "GameEntity.h"
#include "BufferStruct.h"

#include "Graphics.h"

using namespace DirectX;

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat) : mesh(mesh), material(mat)
{
	transform = std::make_shared<Transform>();
}

GameEntity::~GameEntity()
{
}


// Setter
void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh)
{ 
	this->mesh = mesh; 
}

void GameEntity::SetMaterial(std::shared_ptr<Material> mat)
{
}


// Getters
std::shared_ptr<Mesh> GameEntity::GetMesh() 
{ 
	return mesh; 

}
std::shared_ptr<Transform> GameEntity::GetTransform() 
{ 
	return transform; 
}

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return std::shared_ptr<Material>();
}


// Draw Method
void GameEntity::Draw(std::shared_ptr<Camera> camera)
{
	// Constant Buffer Business
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	vs->SetFloat4("colorTint", material->GetColorTint()); // Strings here MUST
	vs->SetMatrix4x4("world", transform->GetWorldMatrix()); // match variable
	vs->SetMatrix4x4("view", camera->GetView()); // names in your
	vs->SetMatrix4x4("projection", camera->GetProjection()); // shader’s cbuffer!
	vs->CopyAllBufferData();

	// Activate our shaders
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	// Draw the mesh using m the mesh draw function
	mesh->Draw();
}
