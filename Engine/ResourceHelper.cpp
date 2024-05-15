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

	//ReadAnimationData(gameObject, scene);

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
			mesh->vertices[v].blendIndices = blendWeight.indices;
			mesh->vertices[v].blendWeights = blendWeight.weights;
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

std::shared_ptr<Animation> ResourceHelper::ReadAnimationData(aiAnimation* srcAnimation, const aiScene* scene)
{
	// ���ο� �ִϸ��̼� ��ü�� �����մϴ�.
	std::shared_ptr<Animation> animation = std::make_shared<Animation>();
	// �ִϸ��̼� �̸��� �����մϴ�.
	animation->name = srcAnimation->mName.C_Str();
	// �ִϸ��̼��� ������ �ӵ��� �����մϴ�. (�ʴ� ƽ ��)
	animation->frameRate = (float)srcAnimation->mTicksPerSecond;
	// �ִϸ��̼��� ������ ���� �����մϴ�. (���� �ð� ���)
	animation->frameCount = (int)srcAnimation->mDuration + 1;

	// �ִϸ��̼� ��带 ĳ���ϱ� ���� ���� �����մϴ�.
	std::map<std::string, std::shared_ptr<AnimationNode>> cacheAnimNodes;
	// Assimp �ִϸ��̼� ä���� ��ȸ�ϸ� ��庰 �ִϸ��̼� �����͸� ó���մϴ�.
	for (int i = 0; i < srcAnimation->mNumChannels; i++) {
		aiNodeAnim* srcNode = srcAnimation->mChannels[i];

		// �ִϸ��̼� ��带 �Ľ��մϴ�.
		std::shared_ptr<AnimationNode> node = ParseAnimationNode(animation, srcNode);

		// �ִϸ��̼��� �ִ� ���� �ð��� ������Ʈ�մϴ�.
		animation->duration = max(animation->duration, node->keyframe.back().time);

		// �Ľ̵� ��带 ĳ�ÿ� �߰��մϴ�.
		cacheAnimNodes[srcNode->mNodeName.C_Str()] = node;
	}
	// �ִϸ��̼� Ű������ �����͸� ó���մϴ�.
	ReadKeyframeData(animation, scene->mRootNode, cacheAnimNodes);

	return animation;
}


// �ִϸ��̼� ��带 �Ľ��ϴ� �Լ�
std::shared_ptr<AnimationNode> ResourceHelper::ParseAnimationNode(std::shared_ptr<Animation> animation, aiNodeAnim* srcNode)
{
	// ���ο� �ִϸ��̼� ��� ��ü�� �����մϴ�.
	std::shared_ptr<AnimationNode> node = std::make_shared<AnimationNode>();
	// ��� �̸��� �����մϴ�.
	node->name = srcNode->mNodeName.C_Str();

	// ��ġ, ȸ��, ������ �� ���� ���� Ű�������� ���� ���� �������� �� Ű������ ���� �����մϴ�.
	int keyCount = max(max(srcNode->mNumPositionKeys, srcNode->mNumScalingKeys), srcNode->mNumRotationKeys);

	// �� Ű�����ӿ� ���� �ݺ��մϴ�.
	for (int k = 0; k < keyCount; k++)
	{

		KeyframeData frameData; // Ű������ ������ ��ü

		bool found = false; // Ű�������� �߰ߵǾ����� ����
		int t = node->keyframe.size(); // ���� Ű�������� �ε���

		// ��ġ, ȸ��, ������ Ű�������� ó���մϴ�.�� Ű�������� �ð��� �����͸� �����Ͽ� frameData�� �����մϴ�.
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
			node->keyframe.push_back(frameData); // ó���� Ű�������� ��忡 �߰��մϴ�.
	}

	// �ִϸ��̼��� Ű������ ������ ����� Ű������ ���� ���� ���, ������ Ű�������� �����Ͽ� ä��ϴ�.
	if (node->keyframe.size() < animation->frameCount) {
		int count = animation->frameCount - node->keyframe.size(); // ä���� �� Ű������ ��
		KeyframeData keyFrame = node->keyframe.back(); // ������ Ű������

		for (int n = 0; n < count; n++) {
			node->keyframe.push_back(keyFrame); // Ű������ �����Ͽ� �߰�
		}
	}

	return node;
}

// �ִϸ��̼� �����Ϳ��� Ư�� ����� Ű������ �����͸� �о� ���� ������ ������ �����ϴ� �Լ�
void ResourceHelper::ReadKeyframeData(std::shared_ptr<Animation> animation, aiNode* node, std::map<std::string, std::shared_ptr<AnimationNode>>& cache)
{
	// ���ο� Ű������ ��ü�� �����մϴ�.
	std::shared_ptr<Keyframe> keyframe = std::make_shared<Keyframe>();
	// ���� ���(��)�� �̸��� Ű�������� �� �̸����� �����մϴ�.
	keyframe->boneName = node->mName.C_Str();

	// ���� ��忡 �ش��ϴ� �ִϸ��̼� ��带 ã���ϴ�.
	std::shared_ptr<AnimationNode> findNode = cache[node->mName.C_Str()];

	// �ִϸ��̼��� ��� �����ӿ� ���� �ݺ��մϴ�.
	for (int i = 0; i < animation->frameCount; i++) {
		KeyframeData frameData; // Ű������ ������ ��ü�� �����մϴ�.

		// ���� ���� ��忡 ���� �ִϸ��̼� ��尡 ĳ�ÿ��� ã������ �ʴ� ���
		if (findNode == nullptr) {
			// ����� ��ȯ ����� ������ ��ġ�� ��, �̸� ������� ��ġ, ȸ��, ������ �����͸� �����մϴ�.
			DirectX::XMMATRIX transform(node->mTransformation[0]);
			transform = XMMatrixTranspose(transform);
			frameData.time = (float)i; // ������ �ð��� �����մϴ�.
			//transform.Decompose(OUT frameData.scale, OUT frameData.rotation, OUT frameData.translation);
			XMMatrixDecompose(&frameData.scale, &frameData.rotation, &frameData.translation, transform);
		}
		else
		{
			// ĳ�ÿ��� ã���� �ִϸ��̼� ��忡 �̹� Ű������ �����Ͱ� ������, �ش� �����͸� ����մϴ�.
			frameData = findNode->keyframe[i];
		}

		// ó���� Ű������ �����͸� Ű������ ��ü�� �߰��մϴ�.
		keyframe->transforms.push_back(frameData);
	}

	// ó���� Ű������ ��ü�� �ִϸ��̼��� Ű������ ��Ͽ� �߰��մϴ�.
	animation->keyframes.push_back(*keyframe);

	// ���� ����� ��� �ڽ� ��忡 ���� ��������� ������ ó���� �����մϴ�.
	for (int i = 0; i < node->mNumChildren; i++)
		ReadKeyframeData(animation, node->mChildren[i], cache);
}