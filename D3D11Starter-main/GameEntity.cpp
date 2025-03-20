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
	// Prepare the material
	material->PrepareMaterial(transform, camera);

	// Draw the mesh using m the mesh draw function
	mesh->Draw();
}
