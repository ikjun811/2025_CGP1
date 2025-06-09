////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"
#include <string> 
#include <wincodec.h> 
#pragma comment(lib, "windowscodecs.lib")

ModelClass::ModelClass()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_Texture = nullptr;
	m_indexCount = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, const WCHAR* modelFilename, const WCHAR* textureFilename)
{

	Assimp::Importer importer;
	std::wstring ws(modelFilename);
	std::string filename_str(ws.begin(), ws.end());

	const aiScene* pScene = importer.ReadFile(filename_str,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		return false;
	}

	aiMatrix4x4 t = pScene->mRootNode->mTransformation;
	m_rootNodeTransform = XMMATRIX(
		t.a1, t.a2, t.a3, t.a4,
		t.b1, t.b2, t.b3, t.b4,
		t.c1, t.c2, t.c3, t.c4,
		t.d1, t.d2, t.d3, t.d4
	);



	// 1. 모델 데이터 로드 
	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		ProcessMesh(pScene->mMeshes[i], pScene);
	}
	ReadNodeHierarchy(pScene->mRootNode, m_skeletonRoot);
	m_finalBoneTransforms.resize(m_boneCounter, XMMatrixIdentity());


	// 2. 텍스처 로드
	if (textureFilename) // 외부 텍스처 파일이 주어지면 그것을 사용
	{
		if (!LoadTexture(device, textureFilename)) return false;
	}
	else // 외부 파일이 없으면 내장 텍스처 로드를 시도
	{
		if (!LoadEmbeddedTexture(device, pScene))
		{
			// 내장 텍스처도 없으면 실패 처리 (또는 기본 텍스처 사용)
			// return false; 
			// 일단은 텍스처 없이 진행하도록 둘 수 있음
		}
	}

	// 3. 정점/인덱스 버퍼 생성
	if (!InitializeBuffers(device))
	{
		return false;
	}

	return true;

}


void ModelClass::Shutdown()
{
	ReleaseTexture();
	ShutdownBuffers();
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}



ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	if (m_Texture)
	{
		return m_Texture->GetTexture();
	}
	return nullptr;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	if (m_vertices.empty() || m_indices.empty())
		return false;

	// 정점 버퍼 설정
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SkinnedVertex) * m_vertices.size(); // SkinnedVertex 사용
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	vertexData.pSysMem = &m_vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result)) return false;

	// 인덱스 버퍼 설정
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	indexData.pSysMem = &m_indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result)) return false;

	m_indexCount = m_indices.size();



	return true;
}

void ModelClass::ShutdownBuffers()
{
	if (m_indexBuffer) { m_indexBuffer->Release(); m_indexBuffer = nullptr; }
	if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(SkinnedVertex);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


bool ModelClass::LoadTexture(ID3D11Device* device, const WCHAR* filename)
{
	m_Texture = new TextureClass;
	if (!m_Texture) return false;
	return m_Texture->Initialize(device, filename);
}

void ModelClass::ReleaseTexture()
{
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}
}
 /*bool ModelClass::LoadModel(const WCHAR* filename)
{
	std::wstring ws(filename);
	std::string filename_str(ws.begin(), ws.end());

	// 파일 확장자를 소문자로 변환하여 비교
	std::string extension = filename_str.substr(filename_str.find_last_of(".") + 1);
	for (char& c : extension) {
		c = tolower(c);
	}

	if (extension == "obj")
	{
		return LoadModelFromObj(filename);
	}
	else
	{
		// .fbx를 포함한 다른 모든 형식은 Assimp로 시도
		return LoadModelWithAssimp(filename);
	}
}*/

/*bool ModelClass::LoadModelWithAssimp(const WCHAR* filename)
{
	Assimp::Importer importer;
	std::wstring ws(filename);
	std::string filename_str(ws.begin(), ws.end());

	const aiScene* pScene = importer.ReadFile(filename_str,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace); // 노멀맵을 위해 탄젠트도 계산

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		// importer.GetErrorString() 으로 에러 로그 출력 추천
		return false;
	}

	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i)
	{
		ProcessMesh(pScene->mMeshes[i], pScene);
	}

	// 스켈레톤 계층 구조 읽기
	ReadNodeHierarchy(pScene->mRootNode, m_skeletonRoot);

	// 최종 뼈 변환 행렬 배열의 크기를 뼈 개수에 맞게 조절
	m_finalBoneTransforms.resize(m_boneCounter, XMMatrixIdentity());

	return true;
}*/


/*void ModelClass::ProcessNode(aiNode* node, const aiScene* scene)
{
	// 현재 노드에 포함된 모든 메쉬를 처리
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene);
	}

	// 자식 노드들을 재귀적으로 처리
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}*/

void ModelClass::ReadNodeHierarchy(const aiNode* pNode, BoneNode& outNode)
{
	outNode.name = pNode->mName.C_Str();
	aiMatrix4x4 t = pNode->mTransformation;
	outNode.transformation = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&t));

	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		BoneNode childNode;
		ReadNodeHierarchy(pNode->mChildren[i], childNode);
		outNode.children.push_back(childNode);
	}
}


void ModelClass::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	// 기존 정점 개수를 기억 (인덱스 오프셋으로 사용)
	int vertexOffset = m_vertices.size();

	// 정점 데이터 채우기
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		SkinnedVertex vertex;
		vertex.Position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		if (mesh->HasNormals())
			vertex.Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		if (mesh->HasTextureCoords(0))
			vertex.TexCoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else
			vertex.TexCoord = XMFLOAT2(0.0f, 0.0f);

		m_vertices.push_back(vertex);
	}

	// 인덱스 데이터 채우기
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			m_indices.push_back(vertexOffset + face.mIndices[j]);
	}

	// 뼈 데이터 처리
	ProcessBones(mesh, vertexOffset);
}


void ModelClass::ProcessBones(aiMesh* mesh, int vertexOffset)
{
	for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		aiBone* bone = mesh->mBones[boneIndex];
		std::string boneName = bone->mName.C_Str();
		int boneID = -1;

		// 이 뼈가 처음 발견된 것이라면, 새로운 ID를 부여하고 맵에 추가
		if (m_boneInfoMap.find(boneName) == m_boneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = m_boneCounter;

			// Assimp 행렬을 DirectX 행렬로 변환
			aiMatrix4x4 t = bone->mOffsetMatrix;
			newBoneInfo.inverseBindPose = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&t));

			m_boneInfoMap[boneName] = newBoneInfo;
			boneID = m_boneCounter;
			m_boneCounter++;
		}
		else
		{
			boneID = m_boneInfoMap[boneName].id;
		}

		// 이 뼈의 영향을 받는 모든 정점들에 ID와 가중치를 기록
		auto weights = bone->mWeights;
		int numWeights = bone->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;

			// m_vertices에서 해당 정점을 찾아 뼈 데이터를 추가
			SkinnedVertex& vertex = m_vertices[vertexOffset + vertexId];

			// 최대 4개의 뼈 가중치를 저장할 자리가 있는지 확인하고 채움
			for (int i = 0; i < 4; ++i)
			{
				if (vertex.BoneWeights.x == 0.0f && i == 0) { vertex.BoneIDs.x = boneID; vertex.BoneWeights.x = weight; break; }
				if (vertex.BoneWeights.y == 0.0f && i == 1) { vertex.BoneIDs.y = boneID; vertex.BoneWeights.y = weight; break; }
				if (vertex.BoneWeights.z == 0.0f && i == 2) { vertex.BoneIDs.z = boneID; vertex.BoneWeights.z = weight; break; }
				if (vertex.BoneWeights.w == 0.0f && i == 3) { vertex.BoneIDs.w = boneID; vertex.BoneWeights.w = weight; break; }
			}
		}
	}
}


// --- 기존 OBJ 로더 구현 ---
bool ModelClass::LoadModelFromObj(const WCHAR* filename)
{
	int vertexCount, textureCount, normalCount, faceCount;
	if (!ReadFileCounts(filename, vertexCount, textureCount, normalCount, faceCount))
	{
		return false;
	}
	return LoadDataStructures(filename, vertexCount, textureCount, normalCount, faceCount);
}


bool ModelClass::ReadFileCounts(const WCHAR* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount)
{
	std::ifstream fin;
	char input;
	vertexCount = 0; textureCount = 0; normalCount = 0; faceCount = 0;

	fin.open(filename);
	if (fin.fail()) return false;

	fin.get(input);
	while (!fin.eof())
	{
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ') vertexCount++;
			if (input == 't') textureCount++;
			if (input == 'n') normalCount++;
		}
		else if (input == 'f')
		{
			fin.get(input);
			if (input == ' ') faceCount++;
		}

		while (input != '\n' && !fin.eof())
			fin.get(input);

		fin.get(input);
	}
	fin.close();
	return true;
}


bool ModelClass::LoadDataStructures(const WCHAR* filename, int vertexCount, int textureCount, int normalCount, int faceCount)
{
	std::vector<XMFLOAT3> temp_vertices(vertexCount);
	std::vector<XMFLOAT2> temp_texcoords(textureCount);
	std::vector<XMFLOAT3> temp_normals(normalCount);
	std::vector<FaceType> faces(faceCount);

	std::ifstream fin;
	char input, input2;
	int vIndex, tIndex, nIndex, fIndex;

	fin.open(filename);
	if (fin.fail()) return false;

	vIndex = 0; tIndex = 0; nIndex = 0; fIndex = 0;

	fin.get(input);
	while (!fin.eof())
	{
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ') { fin >> temp_vertices[vIndex].x >> temp_vertices[vIndex].y >> temp_vertices[vIndex].z; temp_vertices[vIndex].z *= -1.0f; vIndex++; }
			if (input == 't') { fin >> temp_texcoords[tIndex].x >> temp_texcoords[tIndex].y; temp_texcoords[tIndex].y = 1.0f - temp_texcoords[tIndex].y; tIndex++; }
			if (input == 'n') { fin >> temp_normals[nIndex].x >> temp_normals[nIndex].y >> temp_normals[nIndex].z; temp_normals[nIndex].z *= -1.0f; nIndex++; }
		}
		else if (input == 'f')
		{
			fin.get(input);
			if (input == ' ')
			{
				fin >> faces[fIndex].vIndex3 >> input2 >> faces[fIndex].tIndex3 >> input2 >> faces[fIndex].nIndex3
					>> faces[fIndex].vIndex2 >> input2 >> faces[fIndex].tIndex2 >> input2 >> faces[fIndex].nIndex2
					>> faces[fIndex].vIndex1 >> input2 >> faces[fIndex].tIndex1 >> input2 >> faces[fIndex].nIndex1;
				fIndex++;
			}
		}

		while (input != '\n' && !fin.eof())
			fin.get(input);

		fin.get(input);
	}
	fin.close();

	// OBJ 파일은 인덱싱이 복잡하므로, 이전 방식처럼 정점을 중복 생성합니다.
	// (이 부분을 개선하려면 Assimp같은 복잡한 인덱싱 재구성 로직이 필요합니다)
	for (int i = 0; i < faceCount; i++)
	{
		// Vertex 1
		vIndex = faces[i].vIndex1 - 1; tIndex = faces[i].tIndex1 - 1; nIndex = faces[i].nIndex1 - 1;
		m_vertices.push_back({ temp_vertices[vIndex], temp_texcoords[tIndex], temp_normals[nIndex] });

		// Vertex 2
		vIndex = faces[i].vIndex2 - 1; tIndex = faces[i].tIndex2 - 1; nIndex = faces[i].nIndex2 - 1;
		m_vertices.push_back({ temp_vertices[vIndex], temp_texcoords[tIndex], temp_normals[nIndex] });

		// Vertex 3
		vIndex = faces[i].vIndex3 - 1; tIndex = faces[i].tIndex3 - 1; nIndex = faces[i].nIndex3 - 1;
		m_vertices.push_back({ temp_vertices[vIndex], temp_texcoords[tIndex], temp_normals[nIndex] });
	}

	// OBJ 로더는 인덱스 버퍼를 사용하지 않는 것처럼 처리합니다. (정점 중복)
	m_indices.resize(m_vertices.size());
	for (size_t i = 0; i < m_vertices.size(); ++i) {
		m_indices[i] = i;
	}

	return true;
}

bool ModelClass::LoadAnimation(const WCHAR* animationFilename, const std::string& clipName)
{
	Assimp::Importer importer;
	std::wstring ws(animationFilename);
	std::string filename_str(ws.begin(), ws.end());

	const aiScene* pScene = importer.ReadFile(filename_str, aiProcess_Triangulate);
	if (!pScene || !pScene->HasAnimations())
	{
		return false; // 애니메이션이 없으면 실패
	}

	// 이 파일의 첫 번째 애니메이션을 가져옵니다.
	auto pAnimation = pScene->mAnimations[0];
	AnimationClip clip;
	clip.name = clipName;
	clip.duration = (float)pAnimation->mDuration;
	clip.ticksPerSecond = (float)pAnimation->mTicksPerSecond;


	if (clip.ticksPerSecond == 0.0f)
	{
		clip.ticksPerSecond = 30.0f; 
	}



	// 각 뼈(채널)에 대한 애니메이션 데이터를 읽습니다.
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++)
	{
		auto pChannel = pAnimation->mChannels[i];
		std::string boneName = pChannel->mNodeName.C_Str();

		BoneAnimation boneAnim;

		// 위치 키프레임
		for (unsigned int j = 0; j < pChannel->mNumPositionKeys; j++)
		{
			Keyframe<XMFLOAT3> key;
			key.timePos = (float)pChannel->mPositionKeys[j].mTime;
			key.value = XMFLOAT3(pChannel->mPositionKeys[j].mValue.x, pChannel->mPositionKeys[j].mValue.y, pChannel->mPositionKeys[j].mValue.z);
			boneAnim.positionKeys.push_back(key);
		}

		// 회전 키프레임 (쿼터니언)
		for (unsigned int j = 0; j < pChannel->mNumRotationKeys; j++)
		{
			Keyframe<XMFLOAT4> key;
			key.timePos = (float)pChannel->mRotationKeys[j].mTime;
			key.value = XMFLOAT4(pChannel->mRotationKeys[j].mValue.x, pChannel->mRotationKeys[j].mValue.y, pChannel->mRotationKeys[j].mValue.z, pChannel->mRotationKeys[j].mValue.w);
			boneAnim.rotationKeys.push_back(key);
		}

		// 크기 키프레임
		for (unsigned int j = 0; j < pChannel->mNumScalingKeys; j++)
		{
			Keyframe<XMFLOAT3> key;
			key.timePos = (float)pChannel->mScalingKeys[j].mTime;
			key.value = XMFLOAT3(pChannel->mScalingKeys[j].mValue.x, pChannel->mScalingKeys[j].mValue.y, pChannel->mScalingKeys[j].mValue.z);
			boneAnim.scaleKeys.push_back(key);
		}

		clip.boneAnimations[boneName] = boneAnim;
	}

	m_animations[clipName] = clip;

	// 만약 현재 재생중인 애니메이션이 없다면, 방금 로드한 클립을 기본값으로 설정
	if (m_currentAnimation == nullptr)
	{
		SetAnimationClip(clipName);
	}

	return true;
}


void ModelClass::SetAnimationClip(const std::string& clipName)
{
	if (m_animations.count(clipName))
	{
		m_currentAnimation = &m_animations[clipName];
		m_animationTime = 0.0f; // 애니메이션을 처음부터 다시 재생
	}
}

void ModelClass::UpdateAnimation(float deltaTime)
{
	if (!m_currentAnimation) return;

	// 애니메이션 시간 업데이트
	m_animationTime += m_currentAnimation->ticksPerSecond * deltaTime;
	if (m_animationTime > m_currentAnimation->duration)
	{
		m_animationTime = fmod(m_animationTime, m_currentAnimation->duration); // 루프
	}

	// 스켈레톤의 루트부터 시작하여 모든 뼈의 최종 변환 행렬을 계산
	CalculateBoneTransform(m_skeletonRoot, m_rootNodeTransform);
}


void ModelClass::CalculateBoneTransform(const BoneNode& node, const XMMATRIX& parentTransform)
{
	std::string nodeName = node.name;
	XMMATRIX nodeTransform = node.transformation;

	// 이 뼈에 대한 애니메이션 데이터가 있다면, 보간된 변환을 적용
	if (m_currentAnimation->boneAnimations.count(nodeName))
	{
		XMMATRIX scalingMatrix = FindInterpolatedScaling(m_animationTime, nodeName);
		XMMATRIX rotationMatrix = FindInterpolatedRotation(m_animationTime, nodeName);
		XMMATRIX translationMatrix = FindInterpolatedPosition(m_animationTime, nodeName);

		XMMATRIX animationTransform = translationMatrix * rotationMatrix * scalingMatrix;

		nodeTransform = scalingMatrix * rotationMatrix * translationMatrix;
	}

	XMMATRIX globalTransform = nodeTransform * parentTransform;


	if (m_boneInfoMap.count(nodeName))
	{
		int boneIndex = m_boneInfoMap[nodeName].id;
		XMMATRIX inverseBindPose = m_boneInfoMap[nodeName].inverseBindPose;
	
		m_finalBoneTransforms[boneIndex] = inverseBindPose * globalTransform;
	}

	for (const auto& child : node.children)
	{
		CalculateBoneTransform(child, globalTransform);
	}
}

XMMATRIX ModelClass::FindInterpolatedPosition(float animationTime, const std::string& boneName)
{
	const auto& boneAnim = m_currentAnimation->boneAnimations.at(boneName);

	if (boneAnim.positionKeys.size() == 1)
	{
		auto pos = boneAnim.positionKeys[0].value;
		return XMMatrixTranslation(pos.x, pos.y, pos.z);
	}

	// 현재 시간에 맞는 키프레임 인덱스 찾기
	int p0Index = -1;
	for (unsigned int i = 0; i < boneAnim.positionKeys.size() - 1; i++)
	{
		if (animationTime <= boneAnim.positionKeys[i + 1].timePos)
		{
			p0Index = i;
			break;
		}
	}
	if (p0Index == -1) p0Index = boneAnim.positionKeys.size() - 2;
	int p1Index = p0Index + 1;

	// 두 키프레임 사이의 보간 비율 계산
	float t0 = boneAnim.positionKeys[p0Index].timePos;
	float t1 = boneAnim.positionKeys[p1Index].timePos;
	float scaleFactor = (t1 - t0 == 0.0f) ? 0.0f : (animationTime - t0) / (t1 - t0);

	// 선형 보간
	XMVECTOR start = XMLoadFloat3(&boneAnim.positionKeys[p0Index].value);
	XMVECTOR end = XMLoadFloat3(&boneAnim.positionKeys[p1Index].value);
	XMVECTOR finalPos = XMVectorLerp(start, end, scaleFactor);

	return XMMatrixTranslationFromVector(finalPos);
}

XMMATRIX ModelClass::FindInterpolatedRotation(float animationTime, const std::string& boneName)
{
	const auto& boneAnim = m_currentAnimation->boneAnimations.at(boneName);

	if (boneAnim.rotationKeys.size() == 1)
	{
		auto rot = XMLoadFloat4(&boneAnim.rotationKeys[0].value);
		return XMMatrixRotationQuaternion(rot);
	}

	int r0Index = -1;
	for (unsigned int i = 0; i < boneAnim.rotationKeys.size() - 1; i++)
	{
		if (animationTime <= boneAnim.rotationKeys[i + 1].timePos)
		{
			r0Index = i;
			break;
		}
	}
	if (r0Index == -1) r0Index = boneAnim.rotationKeys.size() - 2;
	int r1Index = r0Index + 1;

	float t0 = boneAnim.rotationKeys[r0Index].timePos;
	float t1 = boneAnim.rotationKeys[r1Index].timePos;
	float scaleFactor = (t1 - t0 == 0.0f) ? 0.0f : (animationTime - t0) / (t1 - t0);

	XMVECTOR start = XMLoadFloat4(&boneAnim.rotationKeys[r0Index].value);
	XMVECTOR end = XMLoadFloat4(&boneAnim.rotationKeys[r1Index].value);
	XMVECTOR finalRot = XMQuaternionSlerp(start, end, scaleFactor);
	finalRot = XMQuaternionNormalize(finalRot);

	return XMMatrixRotationQuaternion(finalRot);
}

XMMATRIX ModelClass::FindInterpolatedScaling(float animationTime, const std::string& boneName)
{
	const auto& boneAnim = m_currentAnimation->boneAnimations.at(boneName);

	if (boneAnim.scaleKeys.size() == 1)
	{
		auto scale = boneAnim.scaleKeys[0].value;
		return XMMatrixScaling(scale.x, scale.y, scale.z);
	}

	int s0Index = -1;
	for (unsigned int i = 0; i < boneAnim.scaleKeys.size() - 1; i++)
	{
		if (animationTime <= boneAnim.scaleKeys[i + 1].timePos)
		{
			s0Index = i;
			break;
		}
	}
	if (s0Index == -1) s0Index = boneAnim.scaleKeys.size() - 2;
	int s1Index = s0Index + 1;

	float t0 = boneAnim.scaleKeys[s0Index].timePos;
	float t1 = boneAnim.scaleKeys[s1Index].timePos;
	float scaleFactor = (t1 - t0 == 0.0f) ? 0.0f : (animationTime - t0) / (t1 - t0);

	XMVECTOR start = XMLoadFloat3(&boneAnim.scaleKeys[s0Index].value);
	XMVECTOR end = XMLoadFloat3(&boneAnim.scaleKeys[s1Index].value);
	XMVECTOR finalScale = XMVectorLerp(start, end, scaleFactor);

	return XMMatrixScalingFromVector(finalScale);
}

bool ModelClass::LoadEmbeddedTexture(ID3D11Device* device, const aiScene* scene)
{
	if (!scene->HasTextures())
	{
		return false; // 내장 텍스처가 없음
	}

	// 첫 번째 내장 텍스처를 가져옵니다.
	aiTexture* embeddedTexture = scene->mTextures[0];

	// mHeight가 0이면 압축된 포맷(PNG, JPG 등)입니다.
	if (embeddedTexture->mHeight == 0)
	{
		m_Texture = new TextureClass;
		if (!m_Texture) return false;

		// TextureClass의 새로운 Initialize 함수를 호출합니다.
		return m_Texture->Initialize(device,
			reinterpret_cast<const void*>(embeddedTexture->pcData), // 데이터 포인터
			embeddedTexture->mWidth);                              // 데이터 크기
	}

	// TODO: 압축되지 않은 ARGB 데이터에 대한 처리 (필요 시 구현)

	return false;
}