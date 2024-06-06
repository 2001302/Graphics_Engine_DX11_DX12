#include "BehaviorLeaf.h"
#include "PipelineManager.h"
#include "ImGui/ImGuiManager.h"

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

EnumBehaviorTreeStatus InitializeLightShader::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light shader object.
	manager->LightShader = std::make_unique<LightShader>();

	// Texture sampler 만들기
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

EnumBehaviorTreeStatus InitializePhongShader::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light shader object.
	manager->PhongShader = std::make_unique<PhongShader>();

	// Texture sampler 만들기
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
	Direct3D::GetInstance().GetDevice()->CreateSamplerState(&sampDesc, manager->PhongShader->sampleState.GetAddressOf());

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	manager->PhongShader->CreateVertexShaderAndInputLayout(L"BasicVertexShader.hlsl", inputElements, manager->PhongShader->vertexShader, manager->PhongShader->layout);

	manager->PhongShader->CreatePixelShader(L"BasicPixelShader.hlsl", manager->PhongShader->pixelShader);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjects::Invoke()
{
	IDataBlock* managerBlock = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* envBlock = DataBlock[EnumDataBlockType::eEnv];
	IDataBlock* guiBlock = DataBlock[EnumDataBlockType::eGui];

	auto manager = dynamic_cast<Engine::PipelineManager*>(managerBlock);
	assert(manager != nullptr);

	auto env = dynamic_cast<Engine::Env*>(envBlock);
	assert(env != nullptr);

	auto gui = dynamic_cast<Engine::ImGuiManager*>(guiBlock);
	assert(gui != nullptr);

	auto context = Direct3D::GetInstance().GetDeviceContext();

	// Generate the view matrix based on the camera's position.
	manager->Camera->Render();

	if (gui->m_phongShader)
		return EnumBehaviorTreeStatus::eSuccess;

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

		unsigned int stride = sizeof(Vertex);
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

EnumBehaviorTreeStatus RenderGameObjectsWithPhong::Invoke()
{
	IDataBlock* managerBlock = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* envBlock = DataBlock[EnumDataBlockType::eEnv];
	IDataBlock* guiBlock = DataBlock[EnumDataBlockType::eGui];

	auto manager = dynamic_cast<Engine::PipelineManager*>(managerBlock);
	assert(manager != nullptr);

	auto env = dynamic_cast<Engine::Env*>(envBlock);
	assert(env != nullptr);

	auto gui = dynamic_cast<Engine::ImGuiManager*>(guiBlock);
	assert(gui != nullptr);

	auto context = Direct3D::GetInstance().GetDeviceContext();

	// Generate the view matrix based on the camera's position.
	manager->Camera->Render();

	if (!gui->m_phongShader)
		return EnumBehaviorTreeStatus::eSuccess;

	for (auto& model : manager->Models)
	{
		/*
	    // 모델의 변환
		m_vertexConstantBufferData.model =
        Matrix::CreateScale(m_modelScaling) *
        Matrix::CreateRotationX(m_modelRotation.x) *
        Matrix::CreateRotationY(m_modelRotation.y) *
        Matrix::CreateRotationZ(m_modelRotation.z) *
        Matrix::CreateTranslation(m_modelTranslation);
		*/

		// Transpose the matrices to prepare them for the shader.
		auto modelMatrix = model->transform.Transpose();
		auto viewMatrix = manager->Camera->view.Transpose();

		// Copy the matrices into the constant buffer.
		model->vertexPhongConstantBufferData.model = modelMatrix;
		model->vertexPhongConstantBufferData.view = viewMatrix;

		model->vertexPhongConstantBufferData.invTranspose = model->vertexPhongConstantBufferData.model;
		model->vertexPhongConstantBufferData.invTranspose.Translation(Vector3(0.0f));
		model->vertexPhongConstantBufferData.invTranspose =
			model->vertexPhongConstantBufferData.invTranspose.Transpose().Invert();

		const float aspect = env->aspect;
		if (gui->m_usePerspectiveProjection) {
			model->vertexPhongConstantBufferData.projection = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(gui->m_projFovAngleY), aspect, gui->m_nearZ, gui->m_farZ);
		}
		else {
			model->vertexPhongConstantBufferData.projection = XMMatrixOrthographicOffCenterLH(
				-aspect, aspect, -1.0f, 1.0f, gui->m_nearZ, gui->m_farZ);
		}
		model->vertexPhongConstantBufferData.projection = model->vertexPhongConstantBufferData.projection.Transpose();

		manager->PhongShader->UpdateBuffer(model->vertexPhongConstantBufferData, model->vertexConstantBuffer);

		model->pixelPhongConstantBufferData.eyeWorld = Vector3::Transform(
			Vector3(0.0f), model->vertexPhongConstantBufferData.view.Invert());

		// Copy the lighting variables into the constant buffer.
		model->pixelPhongConstantBufferData.material.diffuse = Vector3(gui->m_materialDiffuse);
		model->pixelPhongConstantBufferData.material.specular = Vector3(gui->m_materialSpecular);
		model->pixelPhongConstantBufferData.material.shininess = gui->m_shininess;

		model->pixelPhongConstantBufferData.useTexture = gui->m_useTexture;
		model->pixelPhongConstantBufferData.useBlinnPhong = gui->m_useBlinnPhong;

		// 여러 개 조명 사용 예시
		for (int i = 0; i < MAX_LIGHTS; i++) {
			// 다른 조명 끄기
			if (i != gui->m_lightType) {
				model->pixelPhongConstantBufferData.lights[i].strength *= 0.0f;
			}
			else {
				model->pixelPhongConstantBufferData.lights[i] = gui->m_lightFromGUI;
			}
		}
		manager->PhongShader->UpdateBuffer(model->pixelPhongConstantBufferData, model->pixelConstantBuffer);

		// RS: Rasterizer stage
		// OM: Output-Merger stage
		// VS: Vertex Shader
		// PS: Pixel Shader
		// IA: Input-Assembler stage

		unsigned int stride = sizeof(Vertex);
		unsigned int offset = 0;

		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context->ClearRenderTargetView(Direct3D::GetInstance().renderTargetView, clearColor);
		context->ClearDepthStencilView(Direct3D::GetInstance().depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		context->OMSetRenderTargets(1, &Direct3D::GetInstance().renderTargetView, Direct3D::GetInstance().depthStencilView.Get());

		context->VSSetShader(manager->PhongShader->vertexShader.Get(), 0, 0);
		context->VSSetConstantBuffers(0, 1, model->vertexConstantBuffer.GetAddressOf());

		context->PSSetShaderResources(0, 1, model->textureResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, &manager->PhongShader->sampleState);
		context->PSSetConstantBuffers(0, 1, model->pixelConstantBuffer.GetAddressOf());
		context->PSSetShader(manager->PhongShader->pixelShader.Get(), NULL, 0);
		context->RSSetState(Direct3D::GetInstance().rasterState.Get());

		context->IASetInputLayout(manager->PhongShader->layout.Get());
		context->IASetVertexBuffers(0, 1, model->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(model->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexed(model->GetIndexCount(), 0, 0);
	}

	return EnumBehaviorTreeStatus::eSuccess;
}