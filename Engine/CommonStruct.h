#ifndef _CommonStruct
#define _CommonStruct

#define _USE_MATH_DEFINES
#define WM_MODEL_LOAD (WM_APP + 1)

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

#include <eigen/dense>
#include <assimp/importer.hpp>
#include <assimp/scene.h>           
#include <assimp/postprocess.h>     

namespace Engine
{
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
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
		std::vector<VertexType> vertices;
		std::vector<int> indices;
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
		float frameRate;//30 �̶�� 1/30�ʸ��� ���� �׸����� �Ѿ
		float duration;
		std::vector<Keyframe> keyframes;// �� �����Ӹ��� ��� ������ Ʋ���ָ� �Ǵ���
	};
	//for cache
	struct AnimationNode
	{
		aiString name;
		std::vector<KeyframeData> keyframe;
	};
}
#endif