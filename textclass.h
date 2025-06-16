////////////////////////////////////////////////////////////////////////////////
// Filename: textclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

#include "fontclass.h"
#include "fontshaderclass.h"
#include <vector> // <<-- vector 헤더 추가

class TextClass
{
private:
	// SentenceType 구조체는 그대로 사용합니다.
	struct SentenceType
	{
		ID3D11Buffer* vertexBuffer, * indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	TextClass();
	TextClass(const TextClass&) = delete;
	TextClass& operator=(const TextClass&) = delete;
	~TextClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd, int screenWidth, int screenHeight, XMMATRIX baseViewMatrix);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX orthoMatrix);

	// FPS와 CPU는 매 프레임 업데이트되므로 별도의 함수로 유지
	bool SetFPS(int fps, ID3D11DeviceContext* deviceContext);
	bool SetCPU(int cpu, ID3D11DeviceContext* deviceContext);

private:
	bool InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device);
	bool UpdateSentence(SentenceType* sentence, const char* text, int positionX, int positionY, float r, float g, float b, ID3D11DeviceContext* deviceContext);
	void ReleaseSentence(SentenceType** sentence);
	bool RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, XMMATRIX worldMatrix, XMMATRIX orthoMatrix);

private:
	FontClass* m_Font;
	FontShaderClass* m_FontShader;
	int m_screenWidth, m_screenHeight;
	XMMATRIX m_baseViewMatrix;

	// <<-- 문장 관리 방식 변경 -->>
	// 동적으로 변하는 문장들 (FPS, CPU)
	SentenceType* m_fpsSentence;
	SentenceType* m_cpuSentence;

	// 고정된 정보성 문장들
	std::vector<SentenceType*> m_infoSentences;
};

#endif