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


using namespace DirectX;

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"



// ASSIMP library
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include "include/assimp/postprocess.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:
	// 이제 모든 모델 데이터는 이 표준 VertexType을 사용합니다.
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

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

	// Initialize는 모델과 텍스처 파일 이름을 받습니다.
	bool Initialize(ID3D11Device* device, const WCHAR* modelFilename, const WCHAR* textureFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	// 버퍼 초기화 및 해제
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	// 텍스처 로딩
	bool LoadTexture(ID3D11Device* device, const WCHAR* filename);
	void ReleaseTexture();

	// --- 모델 로더 선택 및 호출 ---
	bool LoadModel(const WCHAR* filename);

	// 1. Assimp를 사용한 모델 로더
	bool LoadModelWithAssimp(const WCHAR* filename);

	// 2. 기존의 수동 OBJ 모델 로더
	bool LoadModelFromObj(const WCHAR* filename);
	bool ReadFileCounts(const WCHAR* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount);
	bool LoadDataStructures(const WCHAR* filename, int vertexCount, int textureCount, int normalCount, int faceCount);

private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_indexCount; // 이제 m_vertexCount는 m_vertices.size()로 대체 가능

	TextureClass* m_Texture;

	// C-style 배열 대신 std::vector를 사용하여 메모리 관리를 자동화합니다.
	std::vector<VertexType> m_vertices;
	std::vector<unsigned long> m_indices;
};

#endif