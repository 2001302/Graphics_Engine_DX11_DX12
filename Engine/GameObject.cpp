#pragma warning(disable:6385)
#pragma warning(disable:6386)

#include "GameObject.h"

using namespace Engine;

GameObject::GameObject()
{
}

GameObject::GameObject(const GameObject& other)
{
}

GameObject::~GameObject()
{		
	// Release the texture object.
	if (texture)
	{
		texture->Release();
	}

	if (textureResourceView)
	{
		textureResourceView->Release();
	}

	// Shutdown the vertex and index buffers.	
	if (indexBuffer)
	{
		indexBuffer->Release();
	}

	// Release the vertex buffer.
	if (vertexBuffer)
	{
		vertexBuffer->Release();
	}
}

int GameObject::GetIndexCount()
{
	int count = 0;
	for (auto mesh : meshes)
		count += mesh->indices.size();
	return count;
}
