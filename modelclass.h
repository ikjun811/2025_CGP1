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
	// ���� ��� �� �����ʹ� �� ǥ�� VertexType�� ����մϴ�.
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

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
	bool Initialize(ID3D11Device* device, const WCHAR* modelFilename, const WCHAR* textureFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	// ���� �ʱ�ȭ �� ����
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	// �ؽ�ó �ε�
	bool LoadTexture(ID3D11Device* device, const WCHAR* filename);
	void ReleaseTexture();

	// --- �� �δ� ���� �� ȣ�� ---
	bool LoadModel(const WCHAR* filename);

	// 1. Assimp�� ����� �� �δ�
	bool LoadModelWithAssimp(const WCHAR* filename);

	// 2. ������ ���� OBJ �� �δ�
	bool LoadModelFromObj(const WCHAR* filename);
	bool ReadFileCounts(const WCHAR* filename, int& vertexCount, int& textureCount, int& normalCount, int& faceCount);
	bool LoadDataStructures(const WCHAR* filename, int vertexCount, int textureCount, int normalCount, int faceCount);

private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_indexCount; // ���� m_vertexCount�� m_vertices.size()�� ��ü ����

	TextureClass* m_Texture;

	// C-style �迭 ��� std::vector�� ����Ͽ� �޸� ������ �ڵ�ȭ�մϴ�.
	std::vector<VertexType> m_vertices;
	std::vector<unsigned long> m_indices;
};

#endif