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
	manager->camera = std::make_unique<Camera>();

	manager->camera->position = DirectX::SimpleMath::Vector3(0.0f,0.0f,-10.0f);
	manager->camera->Render();

	// Update the position and rotation of the camera for this scene.
	manager->camera->position = DirectX::SimpleMath::Vector3(-200.0f, 0.0f, -200.0f);
	manager->camera->rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializePhongShader::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light shader object.
	manager->phongShader = std::make_unique<PhongShader>();

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
	Direct3D::GetInstance().GetDevice()->CreateSamplerState(&sampDesc, manager->phongShader->sampleState.GetAddressOf());

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	manager->phongShader->CreateVertexShaderAndInputLayout(L"PhongVertexShader.hlsl", inputElements, manager->phongShader->vertexShader, manager->phongShader->layout);

	manager->phongShader->CreatePixelShader(L"PhongPixelShader.hlsl", manager->phongShader->pixelShader);

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
	manager->camera->Render();

	for (auto& model : manager->models)
	{
		//model
		{
			model->phongShader->vertexConstantBufferData.model = Matrix::CreateScale(gui->m_modelScaling) *
				Matrix::CreateRotationX(gui->m_modelRotation.x) *
				Matrix::CreateRotationY(gui->m_modelRotation.y) *
				Matrix::CreateRotationZ(gui->m_modelRotation.z) *
				Matrix::CreateTranslation(gui->m_modelTranslation);

			model->phongShader->vertexConstantBufferData.model = model->phongShader->vertexConstantBufferData.model.Transpose();
		}
		//view
		{
			model->phongShader->vertexConstantBufferData.view = manager->camera->view.Transpose();
		}
		//inverse transpose
		{
			model->phongShader->vertexConstantBufferData.invTranspose = model->phongShader->vertexConstantBufferData.model;
			model->phongShader->vertexConstantBufferData.invTranspose.Translation(Vector3(0.0f));
			model->phongShader->vertexConstantBufferData.invTranspose = model->phongShader->vertexConstantBufferData.invTranspose.Transpose().Invert();
		}
		//projection
		{
			const float aspect = env->aspect;
			if (gui->m_usePerspectiveProjection) {
				model->phongShader->vertexConstantBufferData.projection = XMMatrixPerspectiveFovLH(
					XMConvertToRadians(gui->m_projFovAngleY), aspect, gui->m_nearZ, gui->m_farZ);
			}
			else {
				model->phongShader->vertexConstantBufferData.projection = XMMatrixOrthographicOffCenterLH(
					-aspect, aspect, -1.0f, 1.0f, gui->m_nearZ, gui->m_farZ);
			}
			model->phongShader->vertexConstantBufferData.projection = model->phongShader->vertexConstantBufferData.projection.Transpose();
		}
		
		manager->phongShader->UpdateBuffer(model->phongShader->vertexConstantBufferData, model->phongShader->vertexConstantBuffer);

		//eye
		{
			model->phongShader->pixelConstantBufferData.eyeWorld = Vector3::Transform(
				Vector3(0.0f), model->phongShader->vertexConstantBufferData.view.Invert());
		}
		//material
		{
			model->phongShader->pixelConstantBufferData.material.diffuse = Vector3(gui->m_materialDiffuse);
			model->phongShader->pixelConstantBufferData.material.specular = Vector3(gui->m_materialSpecular);
			model->phongShader->pixelConstantBufferData.material.shininess = gui->m_shininess;
		}
		//light
		{
			for (int i = 0; i < MAX_LIGHTS; i++) 
			{
				if (i != gui->m_lightType) 
				{
					model->phongShader->pixelConstantBufferData.lights[i].strength *= 0.0f;
				}
				else 
				{	
					//turn off another light
					model->phongShader->pixelConstantBufferData.lights[i] = gui->m_lightFromGUI;
				}
			}
		}

		model->phongShader->pixelConstantBufferData.useTexture = gui->m_useTexture;
		model->phongShader->pixelConstantBufferData.useBlinnPhong = gui->m_useBlinnPhong;

		manager->phongShader->UpdateBuffer(model->phongShader->pixelConstantBufferData, model->phongShader->pixelConstantBuffer);

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
		context->OMSetDepthStencilState(Direct3D::GetInstance().depthStencilState.Get(), 0);

		context->VSSetShader(manager->phongShader->vertexShader.Get(), 0, 0);
		context->VSSetConstantBuffers(0, 1, model->phongShader->vertexConstantBuffer.GetAddressOf());

		context->PSSetShaderResources(0, 1, model->textureResourceView.GetAddressOf());
		context->PSSetSamplers(0, 1, &manager->phongShader->sampleState);
		context->PSSetConstantBuffers(0, 1, model->phongShader->pixelConstantBuffer.GetAddressOf());
		context->PSSetShader(manager->phongShader->pixelShader.Get(), NULL, 0);
		
		if(gui->m_drawAsWire)
			context->RSSetState(Direct3D::GetInstance().wireRasterizerSate.Get());
		else 
			context->RSSetState(Direct3D::GetInstance().solidRasterizerSate.Get());

		context->IASetInputLayout(manager->phongShader->layout.Get());
		context->IASetVertexBuffers(0, 1, model->vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(model->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexed(model->GetIndexCount(), 0, 0);
	}

	return EnumBehaviorTreeStatus::eSuccess;
}