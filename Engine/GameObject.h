#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include <d3d11.h>
#include <directxmath.h>

#include "TextureClass.h"
#include "Header.h"

class GameObject
{
private:	
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

public:
	GameObject();
	GameObject(const GameObject&);
	~GameObject();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, const char* textureFilename);
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, std::unique_ptr<TextureClass>& texture);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	DirectX::XMMATRIX Transform;
private:
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename);
	void ReleaseTexture();

	bool LoadModel(const char* filename);
	void ReadModelData(const aiScene* scene, aiNode* node, int index, int parent);
	void ReadMeshData(const aiScene* scene, aiNode* node, int bone);
	void ReadSkinData(const aiScene* scene);
	unsigned int GetBoneIndex(const std::string& name);
	void ReleaseModel();

	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	std::unique_ptr<TextureClass> m_texture;

	std::vector<std::shared_ptr<Bone>> bones;
	std::vector<std::shared_ptr<Mesh>> meshes;
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

	DirectX::XMFLOAT4 indices = DirectX::XMFLOAT4(0, 0, 0, 0); // 쉐이더에 넘겨줄 때도 Vec4로 받아줄 것이기 때문
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
			[weight](const Pair& p) { return weight > p.second; }); // 가장 처음으로 weight이 다른 애 보단 큰 경우 return

		// (1, 0.4) (2, 0, 0.2) 이렇게 있을 때 (5, 0.5)를 넣는다 하면
		// (5, 0.5) (1, 0.4) (2, 0, 0.2) 이렇게 앞에 들어가게 된다. 가중치가 높은 애들을 앞에 배치하기 위해서 이렇게 해줬다.
		boneWeights.insert(findIt, Pair(boneIndex, weight));

	}

	// `asBoneWeights` 구조체는 뼈 인덱스와 가중치 정보를 pair<int32, float>의 형태로 관리합니다.
	// 이 정보는 `asBlendWeight` 구조체를 통해 쉐이더에 호환되는 Vec4 형식으로 변환되어 전달될 예정입니다.
	// 이 변환 과정은 쉐이더에서 데이터를 효율적으로 사용하기 위해 필요합니다.
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

	// 만약에 모든 정점들이 뼈대 4개의 영향을 받으면 좋겠지만, 어떤 애는 하나에만 영향을 받고, 어떤 애는 2개에만 영향을 받게 될 것이다.
	// 결국 최종적인 모든 애들의 합을 1로 맞춰주기 위해 연산을 해준다. 
	// (1, 0.3) (2, 0.2) 이 경우 
	// (1, 0.6) (2, 0.4) 로 해서 최종적인 합을 1로 구해주겠다가 핵심이다. 퍼센티지라고 보면 된다.
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
	std::vector<Pair> boneWeights; // 나중엔 4개 이상이면 정렬해서 가중치 높은 순서로 커트할 것이다.
};

#endif