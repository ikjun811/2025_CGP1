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
	XMFLOAT3 Tangent;

	// 스키닝 데이터 (기본값으로 초기화)
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
	std::map<std::string, BoneAnimation> boneAnimations; // 뼈 이름으로 BoneAnimation을 찾도록 map으로 변경
};

struct BoneNode
{
	std::string name;
	XMMATRIX transformation;
	std::vector<BoneNode> children;
};

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
public:
	
	enum class ShaderType
	{
		Default,  // 정적, 단일 텍스처
		Animated, // 애니메이션, 단일 텍스처
		PBR       // 정적, 다중 텍스처
	};


private:


	// OBJ 파서 전용 내부 구조체들
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

	// 1. 기존 단일 텍스처용 (OBJ, FBX 등)
	bool Initialize(ID3D11Device* device, const WCHAR* modelFilename, const WCHAR* textureFilename = nullptr);
	// 2. 새로운 다중 텍스처용 (PBR 등대 모델용)
	bool Initialize(ID3D11Device* device, const WCHAR* modelFilename, const std::vector<wstring>& textureFilenames);

	bool LoadAnimation(const WCHAR* animationFilename, const std::string& clipName);

	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture(); // 단일 텍스처 반환
	const std::vector<ID3D11ShaderResourceView*>& GetTextures() const; // 다중 텍스처 반환

	ShaderType GetShaderType() const;
	bool HasAnimation() const; // 애니메이션 여부 확인

	// --- 애니메이션 관련 함수
	void UpdateAnimation(float deltaTime);
	void SetAnimationClip(const std::string& clipName);
	const std::vector<XMMATRIX>& GetFinalBoneTransforms() const;

private:
	// 버퍼 초기화 및 해제
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	// 텍스처 로딩
	bool LoadSingleTexture(ID3D11Device* device, const WCHAR* filename);
	bool LoadMultipleTextures(ID3D11Device* device, const vector<wstring>& filenames);
	void ReleaseTextures();

	// --- 모델 로더 선택 및 호출 ---
	//bool LoadModel(const WCHAR* filename);

	 // --- 모델 로더 관련 함수 (기존과 동일) ---
	void ReadNodeHierarchy(const aiNode* pNode, BoneNode& outNode);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void ProcessBones(aiMesh* mesh, int vertexOffset);
	bool LoadEmbeddedTexture(ID3D11Device* device, const aiScene* scene);
	bool LoadModelFromObj(const WCHAR* filename);
	bool ReadFileCounts(const WCHAR* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount);
	bool LoadDataStructures(const WCHAR* filename, int vertexCount, int textureCount, int normalCount, int faceCount);

	// --- 애니메이션 관련 내부 함수 (기존과 동일) ---
	void CalculateBoneTransform(const BoneNode& node, const XMMATRIX& parentTransform);
	XMMATRIX FindInterpolatedPosition(float animationTime, const string& boneName);
	XMMATRIX FindInterpolatedRotation(float animationTime, const string& boneName);
	XMMATRIX FindInterpolatedScaling(float animationTime, const string& boneName);

private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_indexCount;

	TextureClass* m_Texture; // 단일 텍스처용
	vector<TextureClass*> m_TextureList; // 다중 텍스처 클래스 객체 리스트 (메모리 관리용)
	vector<ID3D11ShaderResourceView*> m_Textures; // 다중 텍스처 뷰 벡터 (셰이더 전달용)



	vector<SkinnedVertex> m_vertices;
	vector<unsigned long> m_indices;

	ShaderType m_shaderType;

	// --- 애니메이션 데이터 멤버 ---
	map<string, BoneInfo> m_boneInfoMap;
	int m_boneCounter = 0;
	BoneNode m_skeletonRoot;
	map<string, AnimationClip> m_animations;
	AnimationClip* m_currentAnimation = nullptr;
	float m_animationTime = 0.0f;
	vector<XMMATRIX> m_finalBoneTransforms;
	XMMATRIX m_rootNodeTransform;
};

#endif