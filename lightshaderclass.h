////////////////////////////////////////////////////////////////////////////////
// Filename: lightshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

/////////////
// GLOBALS //
/////////////
const int NUM_LIGHTS = 4;

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <fstream>

#include <vector>

#include "lightclass.h"

#define MAX_BONES 256

using namespace std;
using namespace DirectX;


////////////////////////////////////////////////////////////////////////////////
// Class name: LightShaderClass
////////////////////////////////////////////////////////////////////////////////
class LightShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct BoneBufferType
	{
		XMMATRIX finalBones[MAX_BONES];
	};


	// HLSL의 Light 구조체와 1:1로 대응되는 구조체
	struct Light
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT4 lightPosition;
	};

	// HLSL의 LightBuffer와 1:1로 대응되는 구조체
	struct LightBufferType
	{
		Light lights[NUM_LIGHTS];
	};

public:
	LightShaderClass();
	LightShaderClass(const LightShaderClass&);
	~LightShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	// Render 함수의 인자가 더 간단해집니다.
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		ID3D11ShaderResourceView* texture, const std::vector<LightClass*>& lights,
		const std::vector<XMMATRIX>& boneTransforms);

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* filename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* blob, HWND hwnd, const WCHAR* filename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
		XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		ID3D11ShaderResourceView* texture, const std::vector<LightClass*>& lights,
		const std::vector<XMMATRIX>& boneTransforms);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;

	// 상수 버퍼
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_boneBuffer;
	ID3D11Buffer* m_lightBuffer;
};

#endif