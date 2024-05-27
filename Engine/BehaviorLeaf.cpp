#include "BehaviorLeaf.h"

using namespace Engine;

EnumBehaviorTreeStatus LoadTextureData::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	const char* textureFilename = "../Engine/data/wall01.tga"; //TODO : need path policy
	manager->Texture = std::make_shared<TextureClass>();
	manager->Texture->Initialize(Direct3D::GetInstance().GetDevice(), Direct3D::GetInstance().GetDeviceContext(), textureFilename);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeCamera::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the camera object.
	manager->Camera = std::make_unique<CameraClass>();

	manager->Camera->SetPosition(0.0f, 0.0f, -10.0f);
	manager->Camera->Render();

	// Update the position and rotation of the camera for this scene.
	manager->Camera->SetPosition(-200.0f, 0.0f, -200.0f);
	manager->Camera->SetRotation(0.0f, 45.0f, 0.0f);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeLight::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light object.
	manager->Light = std::make_unique<Light>();

	manager->Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	manager->Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	manager->Light->SetDirection(0.0f, -1.0f, 0.5f);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeShader::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light shader object.
	manager->LightShader = std::make_unique<LightShader>();
	manager->LightShader->Initialize(Direct3D::GetInstance().GetDevice(), m_window);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus GetViewingPoint::Invoke()
{
	IDataBlock* managerBlock = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* viewBlock = DataBlock[EnumDataBlockType::eViewingPoint];

	auto manager = dynamic_cast<Engine::PipelineManager*>(managerBlock);
	assert(manager != nullptr);

	auto viewingPoint = dynamic_cast<Engine::ViewingPoint*>(viewBlock);
	assert(viewingPoint != nullptr);

	// Generate the view matrix based on the camera's position.
	manager->Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	Direct3D::GetInstance().GetWorldMatrix(EnumViewType::eScene, viewingPoint->WorldMatrix);
	manager->Camera->GetViewMatrix(viewingPoint->ViewMatrix);
	Direct3D::GetInstance().GetProjectionMatrix(EnumViewType::eScene, viewingPoint->ProjectionMatrix);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjects::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* viewBlock = DataBlock[EnumDataBlockType::eViewingPoint];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	auto viewingPoint = dynamic_cast<Engine::ViewingPoint*>(viewBlock);
	assert(viewingPoint != nullptr);

	for (auto& model : manager->Models)
	{
		unsigned int stride;
		unsigned int offset;

		// Set vertex buffer stride and offset.
		stride = sizeof(VertexType);
		offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		Direct3D::GetInstance().GetDeviceContext()->IASetVertexBuffers(0, 1, &model->vertexBuffer, &stride, &offset);
		// Set the index buffer to active in the input assembler so it can be rendered.
		Direct3D::GetInstance().GetDeviceContext()->IASetIndexBuffer(model->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		Direct3D::GetInstance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		viewingPoint->WorldMatrix = model->transform;

		HRESULT result;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		LightShader::MatrixBufferType* dataPtr;
		LightShader::LightBufferType* dataPtr2;

		// Transpose the matrices to prepare them for the shader.
		auto worldMatrix = XMMatrixTranspose(viewingPoint->WorldMatrix);
		auto viewMatrix = XMMatrixTranspose(viewingPoint->ViewMatrix);
		auto projectionMatrix = XMMatrixTranspose(viewingPoint->ProjectionMatrix);

		ID3D11ShaderResourceView* texture = model->texture->GetTexture();

		// Lock the constant buffer so it can be written to.
		Direct3D::GetInstance().GetDeviceContext()->Map(manager->LightShader->m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		// Get a pointer to the data in the constant buffer.
		dataPtr = (LightShader::MatrixBufferType*)mappedResource.pData;

		// Copy the matrices into the constant buffer.
		dataPtr->world = worldMatrix;
		dataPtr->view = viewMatrix;
		dataPtr->projection = projectionMatrix;

		// Unlock the constant buffer.
		Direct3D::GetInstance().GetDeviceContext()->Unmap(manager->LightShader->m_matrixBuffer, 0);

		// Set the position of the constant buffer in the vertex shader.
		bufferNumber = 0;

		// Now set the constant buffer in the vertex shader with the updated values.
		Direct3D::GetInstance().GetDeviceContext()->VSSetConstantBuffers(bufferNumber, 1, &manager->LightShader->m_matrixBuffer);
		// Set shader texture resource in the pixel shader.
		Direct3D::GetInstance().GetDeviceContext()->PSSetShaderResources(0, 1, &texture);
		// Lock the light constant buffer so it can be written to.
		Direct3D::GetInstance().GetDeviceContext()->Map(manager->LightShader->m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		// Get a pointer to the data in the constant buffer.
		dataPtr2 = (LightShader::LightBufferType*)mappedResource.pData;

		// Copy the lighting variables into the constant buffer.
		dataPtr2->ambientColor = manager->Light->GetAmbientColor();
		dataPtr2->diffuseColor = manager->Light->GetDiffuseColor();
		dataPtr2->lightDirection = manager->Light->GetDirection();
		dataPtr2->padding = 0.0f;

		// Unlock the constant buffer.
		Direct3D::GetInstance().GetDeviceContext()->Unmap(manager->LightShader->m_lightBuffer, 0);

		// Set the position of the light constant buffer in the pixel shader.
		bufferNumber = 0;

		// Finally set the light constant buffer in the pixel shader with the updated values.
		Direct3D::GetInstance().GetDeviceContext()->PSSetConstantBuffers(bufferNumber, 1, &manager->LightShader->m_lightBuffer);

		// Set the vertex input layout.
		Direct3D::GetInstance().GetDeviceContext()->IASetInputLayout(manager->LightShader->m_layout);

		// Set the vertex and pixel shaders that will be used to render this triangle.
		Direct3D::GetInstance().GetDeviceContext()->VSSetShader(manager->LightShader->m_vertexShader, NULL, 0);
		Direct3D::GetInstance().GetDeviceContext()->PSSetShader(manager->LightShader->m_pixelShader, NULL, 0);

		// Set the sampler state in the pixel shader.
		Direct3D::GetInstance().GetDeviceContext()->PSSetSamplers(0, 1, &manager->LightShader->m_sampleState);

		// Render the triangle.
		Direct3D::GetInstance().GetDeviceContext()->DrawIndexed(model->GetIndexCount(), 0, 0);
	}

	return EnumBehaviorTreeStatus::eSuccess;
}