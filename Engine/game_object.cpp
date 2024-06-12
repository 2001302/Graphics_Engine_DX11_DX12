#pragma warning(disable:6385)
#pragma warning(disable:6386)

#include "game_object.h"

using namespace Engine;

GameObject::GameObject()
{
}

GameObject::GameObject(const GameObject& other)
{
}

GameObject::~GameObject()
{		
}

int GameObject::GetIndexCount()
{
	int count = 0;
	for (auto mesh : meshes)
		count += mesh->indices.size();
	return count;
}
