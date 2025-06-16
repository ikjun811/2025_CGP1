////////////////////////////////////////////////////////////////////////////////
// Filename: billboardshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _BILLBOARDSHADERCLASS_H_
#define _BILLBOARDSHADERCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: BillboardShaderClass
////////////////////////////////////////////////////////////////////////////////
class BillboardShaderClass
{
private:
	// 상수 버퍼는 16바이트 경계에 맞춰야 합니다.
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct BillboardBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding1; // 16바이트 정렬을 위한 패딩
		XMFLOAT3 cameraUp;
		float padding2; // 16바이트 정렬을 위한 패딩
	};

public:
	BillboardShaderClass();
	BillboardShaderClass(const BillboardShaderClass&) = delete;
	BillboardShaderClass& operator=(const BillboardShaderClass&) = delete;
	~BillboardShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
		const XMFLOAT3& cameraPosition, const XMFLOAT3& cameraUp);

private:
	bool InitializeShader(ID3D11Device*, HWND, const WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
		const XMFLOAT3& cameraPosition, const XMFLOAT3& cameraUp);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_billboardBuffer; 
	ID3D11SamplerState* m_sampleState;
};

#endif