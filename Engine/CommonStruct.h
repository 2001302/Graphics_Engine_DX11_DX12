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
}
#endif