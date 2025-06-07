////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"
#include <string> 

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
	// 1. 모델 데이터 로드 
	if (!LoadModel(modelFilename))
	{
		return false;
	}

	// 2. 정점/인덱스 버퍼 생성
	if (!InitializeBuffers(device))
	{
		return false;
	}

	// 3. 텍스처 로드
	if (!LoadTexture(device, textureFilename))
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
	return m_Texture->GetTexture();
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
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertices.size();
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

	// 버퍼 생성 후 CPU 메모리는 비워도 됩니다. (선택사항)
	m_vertices.clear();
	m_indices.clear();

	return true;
}

void ModelClass::ShutdownBuffers()
{
	if (m_indexBuffer) { m_indexBuffer->Release(); m_indexBuffer = nullptr; }
	if (m_vertexBuffer) { m_vertexBuffer->Release(); m_vertexBuffer = nullptr; }
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
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

bool ModelClass::LoadModel(const WCHAR* filename)
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
}

bool ModelClass::LoadModelWithAssimp(const WCHAR* filename)
{
	Assimp::Importer importer;
	std::wstring ws(filename);
	std::string filename_str(ws.begin(), ws.end());

	const aiScene* pScene = importer.ReadFile(filename_str, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);
	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		return false;
	}

	// 이 예제에서는 첫 번째 메시만 로드합니다. (나중에 여러 메시 로드로 확장 가능)
	if (pScene->mNumMeshes == 0) return false;
	aiMesh* pMesh = pScene->mMeshes[0];

	// 정점 데이터 채우기
	m_vertices.resize(pMesh->mNumVertices);
	for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
	{
		m_vertices[i].position = XMFLOAT3(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);

		if (pMesh->HasNormals())
			m_vertices[i].normal = XMFLOAT3(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
		else
			m_vertices[i].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);

		if (pMesh->HasTextureCoords(0))
			m_vertices[i].texture = XMFLOAT2(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
		else
			m_vertices[i].texture = XMFLOAT2(0.0f, 0.0f);
	}

	// 인덱스 데이터 채우기
	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		aiFace face = pMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			m_indices.push_back(face.mIndices[j]);
	}

	return true;
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