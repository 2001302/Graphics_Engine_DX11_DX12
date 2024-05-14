#ifndef _RESOURCEHELPER
#define _RESOURCEHELPER	

#include "GameObject.h"

namespace Engine
{
	/// <summary>
	/// Resource Load/Save/Convert �� �۾��� �����ϴ� class. 
	/// NOTE : member ������ ������ �� ��.
	/// </summary>
	class ResourceHelper
	{
	public:
		ResourceHelper() {};
		~ResourceHelper() {};

		static GameObject* ImportModel(GameObject* gameObject, const char* filename);
		static void ReadModelData(GameObject* gameObject, const aiScene* scene, aiNode* node, int index, int parent);
		static void ReadMeshData(GameObject* gameObject, const aiScene* scene, aiNode* node, int bone);
		static void ReadSkinData(GameObject* gameObject, const aiScene* scene);
		static unsigned int GetBoneIndex(GameObject* gameObject, const std::string& name);
		static void ReadAnimationData(GameObject* gameObject, const aiScene* scene);

		static GameObject* ImportTexture(GameObject* gameObject, ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename);
		static GameObject* ImportTexture(GameObject* gameObject, std::shared_ptr<TextureClass> texture);
	};

	/// <summary>
	/// Animation�� �����ϴ� class
	/// </summary>
	class Animator
	{

	};

	class BlendWeight
	{
	public:
		void Set(unsigned int index, unsigned  int boneIndex, float weight)
		{
			float i = (float)boneIndex;
			float w = weight;

			switch (index)
			{
			case 0: indices.x = i; weights.x = w; break;
			case 1: indices.y = i; weights.y = w; break;
			case 2: indices.z = i; weights.z = w; break;
			case 3: indices.w = i; weights.w = w; break;
			}
		}

		DirectX::XMFLOAT4 indices = DirectX::XMFLOAT4(0, 0, 0, 0); // ���̴��� �Ѱ��� ���� Vec4�� �޾��� ���̱� ����
		DirectX::XMFLOAT4 weights = DirectX::XMFLOAT4(0, 0, 0, 0);
	};
	class BoneWeights
	{
	public:
		void AddWeights(unsigned int boneIndex, float weight)
		{
			if (weight <= 0.0f)
				return;

			auto findIt = std::find_if(boneWeights.begin(), boneWeights.end(),
				[weight](const Pair& p) { return weight > p.second; }); // ���� ó������ weight�� �ٸ� �� ���� ū ��� return

			// (1, 0.4) (2, 0, 0.2) �̷��� ���� �� (5, 0.5)�� �ִ´� �ϸ�
			// (5, 0.5) (1, 0.4) (2, 0, 0.2) �̷��� �տ� ���� �ȴ�. ����ġ�� ���� �ֵ��� �տ� ��ġ�ϱ� ���ؼ� �̷��� �����.
			boneWeights.insert(findIt, Pair(boneIndex, weight));
		}

		// `asBoneWeights` ����ü�� �� �ε����� ����ġ ������ pair<int32, float>�� ���·� �����մϴ�.
		// �� ������ `asBlendWeight` ����ü�� ���� ���̴��� ȣȯ�Ǵ� Vec4 �������� ��ȯ�Ǿ� ���޵� �����Դϴ�.
		// �� ��ȯ ������ ���̴����� �����͸� ȿ�������� ����ϱ� ���� �ʿ��մϴ�.
		BlendWeight GetBlendWeights()
		{
			BlendWeight blendWeights;

			for (unsigned int i = 0; i < boneWeights.size(); i++)
			{
				if (i >= 4)
					break;

				blendWeights.Set(i, boneWeights[i].first, boneWeights[i].second);
			}

			return blendWeights;
		}

		// ���࿡ ��� �������� ���� 4���� ������ ������ ��������, � �ִ� �ϳ����� ������ �ް�, � �ִ� 2������ ������ �ް� �� ���̴�.
		// �ᱹ �������� ��� �ֵ��� ���� 1�� �����ֱ� ���� ������ ���ش�. 
		// (1, 0.3) (2, 0.2) �� ��� 
		// (1, 0.6) (2, 0.4) �� �ؼ� �������� ���� 1�� �����ְڴٰ� �ٽ��̴�. �ۼ�Ƽ����� ���� �ȴ�.
		void Normalize()
		{
			if (boneWeights.size() >= 4)
				boneWeights.resize(4);

			float totalWeight = 0.f;
			for (const auto& item : boneWeights)
				totalWeight += item.second;

			float scale = 1.f / totalWeight;
			for (auto& item : boneWeights)
				item.second *= scale;
		}

		using Pair = std::pair<unsigned int, float>; // boneIndex, weight
		std::vector<Pair> boneWeights; // ���߿� 4�� �̻��̸� �����ؼ� ����ġ ���� ������ ĿƮ�� ���̴�.
	};
}
#endif