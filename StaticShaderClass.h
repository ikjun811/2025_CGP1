////////////////////////////////////////////////////////////////////////////////
// Filename: staticshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _STATICSHADERCLASS_H_
#define _STATICSHADERCLASS_H_

/////////////
// GLOBALS //
/////////////
const int NUM_LIGHTS_STATIC = 4;

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include "lightclass.h"

using namespace std;
using namespace DirectX;



////////////////////////////////////////////////////////////////////////////////
// Class name: StaticShaderClass
////////////////////////////////////////////////////////////////////////////////
class StaticShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct Light
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT4 lightPosition;
	};

	struct LightBufferType
	{
		Light lights[NUM_LIGHTS_STATIC];
	};

public:
	StaticShaderClass();
	StaticShaderClass(const StaticShaderClass&);
	~StaticShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();

	// Render 함수는 뼈 관련 인자가 없습니다.
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
		XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		ID3D11ShaderResourceView* texture, const std::vector<LightClass*>& lights);

private:
	// 셰이더 파일 이름을 staticLight.hlsl로 변경
	bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* filename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* blob, HWND hwnd, const WCHAR* filename);

	// SetShaderParameters 함수도 뼈 관련 인자가 없습니다.
	bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
		XMMATRIX world, XMMATRIX view, XMMATRIX projection,
		ID3D11ShaderResourceView* texture, const std::vector<LightClass*>& lights);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_lightBuffer;
};


#endif