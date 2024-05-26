#include "ResourceHelper.h"

using namespace Engine;

GameObject* ResourceHelper::ImportModel(Engine::GameObject* gameObject, const char* filename)
{
	Assimp::Importer importer;

	auto scene = importer.ReadFile(std::string(filename),
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	/*
		aiProcess_JoinIdenticalVertices |       //Combine identical vertices, optimize indexing
		aiProcess_ValidateDataStructure |       //Verify the output of the loader
		aiProcess_ImproveCacheLocality |        //Improve the cache position of the output vertices
		aiProcess_RemoveRedundantMaterials |    //Remove duplicate materials
		aiProcess_GenUVCoords |					//Convert spherical, cylindrical, box, and planar mapping to appropriate UVs
		aiProcess_TransformUVCoords |           //UV transformation processor (scaling, transforming...)
		aiProcess_FindInstances |               //Search for instanced meshes and remove them as references to a single master
		aiProcess_LimitBoneWeights |            //Limit bone weights to a maximum of four per vertex
		aiProcess_OptimizeMeshes |              //Join small meshes where possible
		aiProcess_GenSmoothNormals |            //Generate smooth normal vectors
		aiProcess_SplitLargeMeshes |            //Split a large single mesh into sub-meshes
		aiProcess_Triangulate |                 //Convert polygonal faces with more than three edges into triangles
		aiProcess_ConvertToLeftHanded |         //Convert to D3D's left-handed coordinate system
		aiProcess_SortByPType);                 //Create a 'clean' mesh composed of a single type of primitive
	*/

	//Using the Assimp library, start from the root node of the imported 3D model and traverse through the model data to read it
	ReadModelData(gameObject, scene, scene->mRootNode, -1, -1);

	//Read the skin (bone) data to be applied to the mesh.
	ReadSkinData(gameObject, scene);

	if(scene->HasAnimations())
	{
		gameObject->animation = ReadAnimationData(scene);
	}
	
	return gameObject;
}

void ResourceHelper::ReadModelData(Engine::GameObject* gameObject, const aiScene* scene, aiNode* node, int index, int parent)
{
	std::shared_ptr<Engine::Bone> bone = std::make_shared<Engine::Bone>();

	bone->index = index;
	bone->parent = parent;
	bone->name = node->mName.C_Str();

	//Retrieve the bone's local transformation matrix, transpose it, and store it
	//Assimp uses column-major matrices, but since DirectX and OpenGL can use row-major matrices, transposing may be necessary.
	DirectX::XMMATRIX transform(node->mTransformation[0]);
	bone->transform = XMMatrixTranspose(transform);

	//Calculate the transformation relative to the root (or parent) bone.
	DirectX::XMMATRIX matParent = DirectX::XMMatrixIdentity();
	if (parent >= 0) {
		matParent = gameObject->bones[parent]->transform;
	}

	//Finally, calculate the transformation matrix of the bone.
	bone->transform = bone->transform * matParent;

	gameObject->bones.push_back(bone);

	//Read the mesh data linked to the current node (bone).
	ReadMeshData(gameObject, scene, node, index);

	//Recursively traverse all child nodes of the current node, repeating the same process.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ReadModelData(gameObject, scene, node->mChildren[i], gameObject->bones.size(), index);
	}
}

void ResourceHelper::ReadMeshData(Engine::GameObject* gameObject, const aiScene* scene, aiNode* node, int bone)
{
	//Do not process nodes without a mesh.
	if (node->mNumMeshes < 1)
		return;

	std::shared_ptr<Engine::Mesh> mesh = std::make_shared<Engine::Mesh>();

	mesh->name = node->mName.C_Str();
	mesh->boneIndex = bone; //Set the bone indices associated with the mesh.

	for (int i = 0; i < node->mNumMeshes; i++) {
		int index = node->mMeshes[i];
		const aiMesh* srcMesh = scene->mMeshes[index];

		const aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();

		for (unsigned int k = 0; k < srcMesh->mNumVertices; k++)
		{
			VertexType vertice;

			DirectX::XMFLOAT3 pos(&srcMesh->mVertices[k].x);
			vertice.position = pos;

			DirectX::XMFLOAT3 normal(&srcMesh->mNormals[k].x);
			vertice.normal = normal;

			if (srcMesh->mTextureCoords[0])
			{
				DirectX::XMFLOAT2 tex = DirectX::XMFLOAT2(&srcMesh->mTextureCoords[0][k].x);
				vertice.texture = tex;
			}

			mesh->vertices.push_back(vertice);
		}

		for (UINT i = 0; i < srcMesh->mNumFaces; ++i)
		{
			const aiFace& face = srcMesh->mFaces[i];

			mesh->indices.push_back(face.mIndices[0]);
			mesh->indices.push_back(face.mIndices[1]);
			mesh->indices.push_back(face.mIndices[2]);
		}
	}

	gameObject->meshes.push_back(mesh);
}

void ResourceHelper::ReadSkinData(Engine::GameObject* gameObject, const aiScene* scene)
{
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* srcMesh = scene->mMeshes[i];

		//Skip if there are no bones
		if (!srcMesh->HasBones()) continue;

		std::shared_ptr<Engine::Mesh> mesh = gameObject->meshes[i];

		std::vector<Engine::BoneWeights> tempVertexBoneWeights;
		tempVertexBoneWeights.resize(mesh->vertices.size());

		for (int b = 0; b < srcMesh->mNumBones; b++) {
			aiBone* srcMeshBone = srcMesh->mBones[b];
			int boneIndex = GetBoneIndex(gameObject, srcMeshBone->mName.C_Str());

			for (unsigned int w = 0; w < srcMeshBone->mNumWeights; w++) {
				unsigned int index = srcMeshBone->mWeights[w].mVertexId;
				float weight = srcMeshBone->mWeights[w].mWeight;

				tempVertexBoneWeights[index].AddWeights(boneIndex, weight);
			}
		}

		for (unsigned int v = 0; v < tempVertexBoneWeights.size(); v++)
		{
			tempVertexBoneWeights[v].Normalize();

			BlendWeight blendWeight = tempVertexBoneWeights[v].GetBlendWeights();
			//mesh->vertices[v].blendIndices = blendWeight.indices;
			//mesh->vertices[v].blendWeights = blendWeight.weights;
		}
	}
}

unsigned int ResourceHelper::GetBoneIndex(Engine::GameObject* gameObject, const std::string& name)
{
	for (std::shared_ptr<Bone>& bone : gameObject->bones)
	{
		if (bone->name == name)
			return bone->index;
	}

	assert(false);
	return 0;
}

GameObject* ResourceHelper::ImportTexture(GameObject* gameObject, ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	gameObject->texture->Initialize(device, deviceContext, filename);
	return gameObject;
}

GameObject* ResourceHelper::ImportTexture(GameObject* gameObject, std::shared_ptr<TextureClass> texture)
{
	gameObject->texture = texture;
	return gameObject;
}

std::shared_ptr<Animation> ResourceHelper::ReadAnimationData(const aiScene* scene)
{
	auto srcAnimation = *scene->mAnimations;
	// 새로운 애니메이션 객체를 생성합니다.
	std::shared_ptr<Animation> animation = std::make_shared<Animation>();
	// 애니메이션 이름을 설정합니다.
	animation->name = srcAnimation->mName.C_Str();
	// 애니메이션의 프레임 속도를 설정합니다. (초당 틱 수)
	animation->frameRate = (float)srcAnimation->mTicksPerSecond;
	// 애니메이션의 프레임 수를 설정합니다. (지속 시간 기반)
	animation->frameCount = (int)srcAnimation->mDuration + 1;

	// 애니메이션 노드를 캐싱하기 위한 맵을 선언합니다.
	std::map<std::string, std::shared_ptr<AnimationNode>> cacheAnimNodes;
	// Assimp 애니메이션 채널을 순회하며 노드별 애니메이션 데이터를 처리합니다.
	for (int i = 0; i < srcAnimation->mNumChannels; i++) {
		aiNodeAnim* srcNode = srcAnimation->mChannels[i];

		// 애니메이션 노드를 파싱합니다.
		std::shared_ptr<AnimationNode> node = ParseAnimationNode(animation, srcNode);

		// 애니메이션의 최대 지속 시간을 업데이트합니다.
		animation->duration = max(animation->duration, node->keyframe.back().time);

		// 파싱된 노드를 캐시에 추가합니다.
		cacheAnimNodes[srcNode->mNodeName.C_Str()] = node;
	}
	// 애니메이션 키프레임 데이터를 처리합니다.
	ReadKeyframeData(animation, scene->mRootNode, cacheAnimNodes);

	return animation;
}


// 애니메이션 노드를 파싱하는 함수
std::shared_ptr<AnimationNode> ResourceHelper::ParseAnimationNode(std::shared_ptr<Animation> animation, aiNodeAnim* srcNode)
{
	// 새로운 애니메이션 노드 객체를 생성합니다.
	std::shared_ptr<AnimationNode> node = std::make_shared<AnimationNode>();
	// 노드 이름을 설정합니다.
	node->name = srcNode->mNodeName.C_Str();

	// 위치, 회전, 스케일 중 가장 많은 키프레임을 가진 것을 기준으로 총 키프레임 수를 결정합니다.
	int keyCount = max(max(srcNode->mNumPositionKeys, srcNode->mNumScalingKeys), srcNode->mNumRotationKeys);

	// 각 키프레임에 대해 반복합니다.
	for (int k = 0; k < keyCount; k++)
	{

		KeyframeData frameData; // 키프레임 데이터 객체

		bool found = false; // 키프레임이 발견되었는지 여부
		int t = node->keyframe.size(); // 현재 키프레임의 인덱스

		// 위치, 회전, 스케일 키프레임을 처리합니다.각 키프레임의 시간과 데이터를 추출하여 frameData에 저장합니다.
		// Position
		if (::fabsf((float)srcNode->mPositionKeys[k].mTime - (float)t) <= 0.0001f)
		{
			aiVectorKey key = srcNode->mPositionKeys[k];
			frameData.time = (float)key.mTime;
			::memcpy_s(&frameData.translation, sizeof(DirectX::XMFLOAT3), &key.mValue, sizeof(aiVector3D));

			found = true;
		}

		// Rotation
		if (::fabsf((float)srcNode->mRotationKeys[k].mTime - (float)t) <= 0.0001f)
		{
			aiQuatKey key = srcNode->mRotationKeys[k];
			frameData.time = (float)key.mTime;
			frameData.rotation = { key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w };

			found = true;
		}

		// Scale
		if (::fabsf((float)srcNode->mScalingKeys[k].mTime - (float)t) <= 0.0001f)
		{
			aiVectorKey key = srcNode->mScalingKeys[k];
			frameData.time = (float)key.mTime;
			::memcpy_s(&frameData.scale, sizeof(DirectX::XMFLOAT3), &key.mValue, sizeof(aiVector3D));

			found = true;
		}

		if (found == true)
			node->keyframe.push_back(frameData); // 처리된 키프레임을 노드에 추가합니다.
	}

	// 애니메이션의 키프레임 수보다 노드의 키프레임 수가 적은 경우, 마지막 키프레임을 복제하여 채웁니다.
	if (node->keyframe.size() < animation->frameCount) {
		int count = animation->frameCount - node->keyframe.size(); // 채워야 할 키프레임 수
		KeyframeData keyFrame = node->keyframe.back(); // 마지막 키프레임

		for (int n = 0; n < count; n++) {
			node->keyframe.push_back(keyFrame); // 키프레임 복제하여 추가
		}
	}

	return node;
}

// 애니메이션 데이터에서 특정 노드의 키프레임 데이터를 읽어 내부 데이터 구조에 저장하는 함수
void ResourceHelper::ReadKeyframeData(std::shared_ptr<Animation> animation, aiNode* node, std::map<std::string, std::shared_ptr<AnimationNode>>& cache)
{
	// 새로운 키프레임 객체를 생성합니다.
	std::shared_ptr<Keyframe> keyframe = std::make_shared<Keyframe>();
	// 현재 노드(본)의 이름을 키프레임의 본 이름으로 설정합니다.
	keyframe->boneName = node->mName.C_Str();

	// 현재 노드에 해당하는 애니메이션 노드를 찾습니다.
	std::shared_ptr<AnimationNode> findNode = cache[node->mName.C_Str()];

	// 애니메이션의 모든 프레임에 대해 반복합니다.
	for (int i = 0; i < animation->frameCount; i++) {
		KeyframeData frameData; // 키프레임 데이터 객체를 생성합니다.

		// 만약 현재 노드에 대한 애니메이션 노드가 캐시에서 찾아지지 않는 경우
		if (findNode == nullptr) {
			// 노드의 변환 행렬을 가져와 전치한 뒤, 이를 기반으로 위치, 회전, 스케일 데이터를 추출합니다.
			DirectX::XMMATRIX transform(node->mTransformation[0]);
			transform = XMMatrixTranspose(transform);
			frameData.time = (float)i; // 프레임 시간을 설정합니다.
			//transform.Decompose(OUT frameData.scale, OUT frameData.rotation, OUT frameData.translation);
			XMMatrixDecompose(&frameData.scale, &frameData.rotation, &frameData.translation, transform);
		}
		else
		{
			// 캐시에서 찾아진 애니메이션 노드에 이미 키프레임 데이터가 있으면, 해당 데이터를 사용합니다.
			frameData = findNode->keyframe[i];
		}

		// 처리된 키프레임 데이터를 키프레임 객체에 추가합니다.
		keyframe->transforms.push_back(frameData);
	}

	// 처리된 키프레임 객체를 애니메이션의 키프레임 목록에 추가합니다.
	animation->keyframes.push_back(*keyframe);

	// 현재 노드의 모든 자식 노드에 대해 재귀적으로 동일한 처리를 수행합니다.
	for (int i = 0; i < node->mNumChildren; i++)
		ReadKeyframeData(animation, node->mChildren[i], cache);
}