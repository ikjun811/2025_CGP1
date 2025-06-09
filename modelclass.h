////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

/////////////
// LINKING //
/////////////
#pragma comment(lib, "lib/assimp-vc143-mt.lib")



//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <fstream>
#include <vector> 
#include <string> 
#include <map>    

using namespace DirectX;

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"



// ASSIMP library
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include "include/assimp/postprocess.h"


struct SkinnedVertex
{
	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Normal;

	// ��Ű�� ������ (�⺻������ �ʱ�ȭ)
	XMUINT4  BoneIDs = { 0,0,0,0 };
	XMFLOAT4 BoneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct BoneInfo
{
	int id;
	XMMATRIX inverseBindPose;
};

template<typename T>
struct Keyframe
{
	float timePos;
	T value;
};

struct BoneAnimation
{
	std::vector<Keyframe<XMFLOAT3>> positionKeys;
	std::vector<Keyframe<XMFLOAT4>> rotationKeys;
	std::vector<Keyframe<XMFLOAT3>> scaleKeys;
};

struct AnimationClip
{
	std::string name;
	float duration;
	float ticksPerSecond;
	std::map<std::string, BoneAnimation> boneAnimations; // �� �̸����� BoneAnimation�� ã���� map���� ����
};

struct BoneNode
{
	std::string name;
	XMMATRIX transformation;
	std::vector<BoneNode> children;
};


////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:


	// OBJ �ļ� ���� ���� ����ü��
	struct FaceType
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	// Initialize�� �𵨰� �ؽ�ó ���� �̸��� �޽��ϴ�.
	bool Initialize(ID3D11Device* device, const WCHAR* modelFilename, const WCHAR* textureFilename = nullptr);
	
	bool LoadAnimation(const WCHAR* animationFilename, const std::string& clipName);

	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	void UpdateAnimation(float deltaTime);
	void SetAnimationClip(const std::string& clipName);
	const std::vector<XMMATRIX>& GetFinalBoneTransforms() const { return m_finalBoneTransforms; }

private:
	// ���� �ʱ�ȭ �� ����
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	// �ؽ�ó �ε�
	bool LoadTexture(ID3D11Device* device, const WCHAR* filename);
	void ReleaseTexture();

	// --- �� �δ� ���� �� ȣ�� ---
	//bool LoadModel(const WCHAR* filename);

	// 1. Assimp�� ����� �� �δ�
	//bool LoadModelWithAssimp(const WCHAR* filename);
	//void ProcessNode(aiNode* node, const aiScene* scene);
	void ReadNodeHierarchy(const aiNode* pNode, BoneNode& outNode);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void ProcessBones(aiMesh* mesh, int vertexOffset);
	bool LoadEmbeddedTexture(ID3D11Device* device, const aiScene* scene);

	// 2. ������ ���� OBJ �� �δ�
	bool LoadModelFromObj(const WCHAR* filename);
	bool ReadFileCounts(const WCHAR* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount);
	bool LoadDataStructures(const WCHAR* filename, int vertexCount, int textureCount, int normalCount, int faceCount);

	// --- �ִϸ��̼� ���� ���� �Լ� ---
	void CalculateBoneTransform(const BoneNode& node, const XMMATRIX& parentTransform);
	XMMATRIX FindInterpolatedPosition(float animationTime, const std::string& boneName);
	XMMATRIX FindInterpolatedRotation(float animationTime, const std::string& boneName);
	XMMATRIX FindInterpolatedScaling(float animationTime, const std::string& boneName);

private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_indexCount;

	TextureClass* m_Texture;

	std::vector<SkinnedVertex> m_vertices; 
	std::vector<unsigned long> m_indices;

	// --- �ִϸ��̼� ������ ��� ---
	std::map<std::string, BoneInfo> m_boneInfoMap;
	int m_boneCounter = 0;

	BoneNode m_skeletonRoot;

	std::map<std::string, AnimationClip> m_animations;
	AnimationClip* m_currentAnimation = nullptr;
	float m_animationTime = 0.0f;

	std::vector<XMMATRIX> m_finalBoneTransforms;

	XMMATRIX m_rootNodeTransform;

};

#endif