#include "BehaviorLeaf.h"

using namespace Engine;

EnumBehaviorTreeStatus InitializeCamera::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the camera object.
	manager->Camera = std::make_unique<Camera>();

	manager->Camera->position = DirectX::SimpleMath::Vector3(0.0f,0.0f,-10.0f);
	manager->Camera->Render();

	// Update the position and rotation of the camera for this scene.
	manager->Camera->position = DirectX::SimpleMath::Vector3(-200.0f, 0.0f, -200.0f);
	manager->Camera->rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

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

	// Texture sampler ¸¸µé±â
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the Sample State
	Direct3D::GetInstance().GetDevice()->CreateSamplerState(&sampDesc, manager->LightShader->sampleState.GetAddressOf());

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	manager->LightShader->CreateVertexShaderAndInputLayout(L"LightVertexShader.hlsl", inputElements, manager->LightShader->vertexShader, manager->LightShader->layout);

	manager->LightShader->CreatePixelShader(L"LightPixelShader.hlsl", manager->LightShader->pixelShader);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjects::Invoke()
{
	IDataBlock* managerBlock = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* envBlock = DataBlock[EnumDataBlockType::eEnv];

	auto manager = dynamic_cast<Engine::PipelineManager*>(managerBlock);
	assert(manager != nullptr);

	auto env = dynamic_cast<Engine::Env*>(envBlock);
	assert(env != nullptr);

	auto context = Direct3D::GetInstance().GetDeviceContext();

	// Generate the view matrix based on the camera's position.
	manager->Camera->Render();

	for (auto& model : manager->Models)
	{
		// Transpose the matrices to prepare them for the shader.
		auto worldMatrix = XMMatrixTranspose(model->transform);
		auto viewMatrix = XMMatrixTranspose(manager->Camera->view);
		auto projectionMatrix = XMMatrixTranspose(XMMatrixPerspectiveFovLH(env->fieldOfView, env->aspect, env->screenNear, env->screenDepth));

		// Copy the matrices into the constant buffer.
		model->vertexConstantBufferData.world = worldMatrix;
		model->vertexConstantBufferData.view = viewMatrix;
		model->vertexConstantBufferData.projection = projectionMatrix;

		// Copy the lighting variables into the constant buffer.
		model->pixelConstantBufferData.ambientColor = manager->Light->GetAmbientColor();
		model->pixelConstantBufferData.diffuseColor = manager->Light->GetDiffuseColor();
		model->pixelConstantBufferData.lightDirection = manager->Light->GetDirection();
		model->pixelConstantBufferData.padding = 0.0f;

		manager->LightShader->UpdateBuffer(model->vertexConstantBufferData, model->vertexConstantBuffer);
		manager->LightShader->UpdateBuffer(model->pixelConstantBufferData, model->pixelConstantBuffer);

		// RS: Rasterizer stage
		// OM: Output-Merger stage
		// VS: Vertex Shader
		// PS: Pixel Shader
		// IA: Input-Assembler stage

		unsigned int stride = sizeof(VertexType);
		unsigned int offset = 0;

		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context->ClearRenderTargetView(Direct3D::GetInstance().renderTargetView, clearColor);
		context->ClearDepthStencilView(Direct3D::GetInstance().depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(1, &Direct3D::GetInstance().renderTargetView, Direct3D::GetInstance().depthStencilView.Get());

		context->VSSetShader(manager->LightShader->vertexShader.Get(), 0, 0);
		context->VSSetConstantBuffers(0, 1, model->vertexConstantBuffer.GetAddressOf());

		context->PSSetShaderResources(0, 1, model->textureResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, &manager->LightShader->sampleState);
		context->PSSetConstantBuffers(0, 1, model->pixelConstantBuffer.GetAddressOf());
		context->PSSetShader(manager->LightShader->pixelShader.Get(), NULL, 0);
		context->RSSetState(Direct3D::GetInstance().rasterState.Get());

		context->IASetInputLayout(manager->LightShader->layout.Get());
		context->IASetVertexBuffers(0, 1, model->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(model->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexed(model->GetIndexCount(), 0, 0);
	}

	return EnumBehaviorTreeStatus::eSuccess;
}