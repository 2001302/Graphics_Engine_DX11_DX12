#ifndef _CommonStruct
#define _CommonStruct

#include "Header.h"

namespace Engine
{
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 blendIndices;
		DirectX::XMFLOAT4 blendWeights;
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