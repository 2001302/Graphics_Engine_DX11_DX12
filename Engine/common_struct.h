#ifndef _CommonStruct
#define _CommonStruct

#define _USE_MATH_DEFINES
#define WM_MODEL_LOAD (WM_APP + 1)
#define WM_BOX_LOAD (WM_APP + 2)
#define WM_CYLINDER_LOAD (WM_APP + 3)
#define WM_SPHERE_LOAD (WM_APP + 4)

#include <map>
#include <vector>
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>		//for text file open
#include <cmath>		//for calculate coordination
#include <omp.h>		//for omp parallel
#include <commdlg.h>	//for file open
#include <wrl.h>

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

#include <assimp/importer.hpp>
#include <assimp/scene.h>           
#include <assimp/postprocess.h>     

namespace Engine
{
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector4;
	using Microsoft::WRL::ComPtr;

    enum EnumShaderType {
        ePhong = 0,
        eCube = 1,
        eImageBased = 2,
    };

	struct Vertex {
		Vector3 position;
		Vector3 normal;
		Vector2 texcoord;
		//Vector3 color;
	};

	struct Bone
	{
		int index;
		std::string name;
		int parent;
		DirectX::XMMATRIX transform;
	};

	struct Mesh
	{
		std::string name;
		int boneIndex;
		std::string materialName;
		std::string textureFilename;
		std::vector<Vertex> vertices;
		std::vector<int> indices;

		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> textureResourceView;
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
	};

	struct KeyframeData
	{
		float time;
		DirectX::XMVECTOR scale;
		DirectX::XMVECTOR rotation;
		DirectX::XMVECTOR translation;
	};

	//a frame of animation
	struct Keyframe
	{
		std::string boneName;
		std::vector<KeyframeData> transforms;
	};

	//contains information on how the joints change in each frame.
	struct Animation
	{
		std::string name;
		int frameCount;
		float frameRate;//30 이라면 1/30초마다 다음 그림으로 넘어감
		float duration;
		std::vector<Keyframe> keyframes;// 매 프레임마다 어떠한 정보로 틀어주면 되는지
	};

	//for cache
	struct AnimationNode
	{
		aiString name;
		std::vector<KeyframeData> keyframe;
	};

	struct Material {
		Vector3 ambient = Vector3(0.1f);  // 12
		float shininess = 1.0f;           // 4
		Vector3 diffuse = Vector3(0.5f);  // 12
		float dummy1;                     // 4
		Vector3 specular = Vector3(0.5f); // 12
		float dummy2;                     // 4
	};

	struct Light {
		Vector3 strength = Vector3(1.0f);              // 12
		float fallOffStart = 0.0f;                     // 4
		Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // 12
		float fallOffEnd = 10.0f;                      // 4
		Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // 12
		float spotPower = 1.0f;                        // 4
	};

}
#endif