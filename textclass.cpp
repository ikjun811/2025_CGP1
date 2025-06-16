////////////////////////////////////////////////////////////////////////////////
// Filename: textclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textclass.h"

TextClass::TextClass()
{
	m_Font = nullptr;
	m_FontShader = nullptr;
	m_fpsSentence = nullptr;
	m_cpuSentence = nullptr;
}

TextClass::~TextClass()
{
}

bool TextClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd,
	int screenWidth, int screenHeight, XMMATRIX baseViewMatrix)
{
	bool result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_baseViewMatrix = baseViewMatrix;

	// 폰트 및 폰트 셰이더 초기화
	m_Font = new FontClass;
	if (!m_Font) return false;
	result = m_Font->Initialize(device, L"./data/fontdata.txt", L"./data/font.dds");
	if (!result) { MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK); return false; }

	m_FontShader = new FontShaderClass;
	if (!m_FontShader) return false;
	result = m_FontShader->Initialize(device, hwnd);
	if (!result) { MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK); return false; }

	// --- 동적 문장들(FPS, CPU) 초기화 ---
	result = InitializeSentence(&m_fpsSentence, 16, device);
	if (!result) return false;
	result = InitializeSentence(&m_cpuSentence, 16, device);
	if (!result) return false;

	// 초기값 설정 (선택 사항)
	result = UpdateSentence(m_fpsSentence, "FPS = 0", 20, 20, 1.f, 1.f, 1.f, deviceContext);
	if (!result) return false;
	result = UpdateSentence(m_cpuSentence, "CPU = 0%", 20, 40, 1.f, 1.f, 1.f, deviceContext);
	if (!result) return false;

	// --- 고정 정보 문장들 초기화 및 설정 ---
	// 헬퍼 람다 함수를 사용하여 코드 중복 줄이기
	auto CreateInfoSentence = [&](const char* text, int x, int y) -> bool {
		SentenceType* newSentence = nullptr;
		if (!InitializeSentence(&newSentence, strlen(text) + 1, device)) return false;
		if (!UpdateSentence(newSentence, text, x, y, 1.0f, 1.0f, 1.0f, deviceContext)) return false; // 노란색으로 설정
		m_infoSentences.push_back(newSentence);
		return true;
		};

	if (!CreateInfoSentence("C093196 LEE IKJUN", 20, 70)) return false;
	if (!CreateInfoSentence("1920 1080", 20, 100)) return false;
	if (!CreateInfoSentence("Lighthouse Of Choice", 20, 130)) return false;
	if (!CreateInfoSentence("FirstPersonCamera", 20, 160)) return false;
	if (!CreateInfoSentence("ASSIMP DirectXSDK", 20, 190)) return false;
	if (!CreateInfoSentence("MultiTexturing by Time", 20, 220)) return false;
	if (!CreateInfoSentence("Collision Detection", 20, 250)) return false;
	if (!CreateInfoSentence("Raycast", 20, 280)) return false;
	if (!CreateInfoSentence("Animation Fail", 20, 310)) return false;

	return true;
}

void TextClass::Shutdown()
{
	// 동적 문장들 해제
	ReleaseSentence(&m_fpsSentence);
	ReleaseSentence(&m_cpuSentence);

	// 고정 정보 문장들 해제
	for (auto& sentence : m_infoSentences)
	{
		ReleaseSentence(&sentence);
	}
	m_infoSentences.clear();

	// 폰트 셰이더 및 폰트 객체 해제
	if (m_FontShader) { m_FontShader->Shutdown(); delete m_FontShader; m_FontShader = nullptr; }
	if (m_Font) { m_Font->Shutdown(); delete m_Font; m_Font = nullptr; }
}

bool TextClass::Render(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{
	// 동적 문장들 렌더링
	RenderSentence(deviceContext, m_fpsSentence, worldMatrix, orthoMatrix);
	RenderSentence(deviceContext, m_cpuSentence, worldMatrix, orthoMatrix);

	// 고정 정보 문장들 렌더링
	for (const auto& sentence : m_infoSentences)
	{
		RenderSentence(deviceContext, sentence, worldMatrix, orthoMatrix);
	}

	return true;
}

// <<-- UpdateSentence 함수 성능 개선 -->>
bool TextClass::UpdateSentence(SentenceType* sentence, const char* text, int positionX, int positionY,
	float red, float green, float blue, ID3D11DeviceContext* deviceContext)
{
	int numLetters;
	VertexType* vertices;
	float drawX, drawY;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	numLetters = (int)strlen(text);
	if (numLetters > sentence->maxLength) return false;

	vertices = new VertexType[sentence->vertexCount];
	if (!vertices) return false;
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	drawY = (float)((m_screenHeight / 2) - positionY);
	m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		delete[] vertices;
		return false;
	}

	verticesPtr = (VertexType*)mappedResource.pData;
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));
	deviceContext->Unmap(sentence->vertexBuffer, 0);

	delete[] vertices;
	vertices = nullptr;

	return true;
}


// --- 나머지 함수들은 기존 코드와 동일 ---
// InitializeSentence, ReleaseSentence, RenderSentence, SetFPS, SetCPU
// (이 함수들은 수정할 필요가 없습니다.)

bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	*sentence = new SentenceType;
	if (!*sentence) return false;

	(*sentence)->vertexBuffer = nullptr;
	(*sentence)->indexBuffer = nullptr;
	(*sentence)->maxLength = maxLength;
	(*sentence)->vertexCount = 6 * maxLength;
	(*sentence)->indexCount = (*sentence)->vertexCount;

	vertices = new VertexType[(*sentence)->vertexCount];
	if (!vertices) return false;
	indices = new unsigned long[(*sentence)->indexCount];
	if (!indices) { delete[] vertices; return false; }

	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));
	for (int i = 0; i < (*sentence)->indexCount; i++) { indices[i] = i; }

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if (FAILED(result)) { delete[] vertices; delete[] indices; return false; }

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if (FAILED(result)) { delete[] vertices; delete[] indices; return false; }

	delete[] vertices;
	vertices = nullptr;
	delete[] indices;
	indices = nullptr;

	return true;
}

void TextClass::ReleaseSentence(SentenceType** sentence)
{
	if (*sentence)
	{
		if ((*sentence)->vertexBuffer) { (*sentence)->vertexBuffer->Release(); (*sentence)->vertexBuffer = nullptr; }
		if ((*sentence)->indexBuffer) { (*sentence)->indexBuffer->Release(); (*sentence)->indexBuffer = nullptr; }
		delete* sentence;
		*sentence = nullptr;
	}
}

bool TextClass::RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, XMMATRIX worldMatrix, XMMATRIX orthoMatrix)
{
	if (!sentence) return true; // 문장이 없으면 그냥 성공 처리

	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMFLOAT4 pixelColor = XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);

	if (!m_FontShader->Render(deviceContext, sentence->indexCount, worldMatrix, m_baseViewMatrix, orthoMatrix, m_Font->GetTexture(), pixelColor))
	{
		return false;
	}

	return true;
}

bool TextClass::SetFPS(int fps, ID3D11DeviceContext* deviceContext)
{
	char tempString[16], fpsString[16];
	float red, green, blue;

	if (fps > 9999) fps = 9999;
	_itoa_s(fps, tempString, 10);
	strcpy_s(fpsString, "FPS = ");
	strcat_s(fpsString, tempString);

	if (fps >= 60) { red = 0.0f; green = 1.0f; blue = 0.0f; }
	else if (fps < 60 && fps >= 30) { red = 1.0f; green = 1.0f; blue = 0.0f; }
	else { red = 1.0f; green = 0.0f; blue = 0.0f; }

	return UpdateSentence(m_fpsSentence, fpsString, 20, 20, red, green, blue, deviceContext);
}

bool TextClass::SetCPU(int cpu, ID3D11DeviceContext* deviceContext)
{
	char tempString[16], cpuString[16];

	_itoa_s(cpu, tempString, 10);
	strcpy_s(cpuString, "CPU = ");
	strcat_s(cpuString, tempString);
	strcat_s(cpuString, "%");

	return UpdateSentence(m_cpuSentence, cpuString, 20, 40, 0.0f, 1.0f, 0.0f, deviceContext);
}