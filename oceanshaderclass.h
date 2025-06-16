#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: oceanshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _OCEANSHADERCLASS_H_
#define _OCEANSHADERCLASS_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>
#include "lightclass.h"
#include "cameraclass.h"

using namespace std;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: OceanShaderClass
////////////////////////////////////////////////////////////////////////////////
class OceanShaderClass
{
private:
	// 상수 버퍼 구조체들
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	// Light 구조체는 HLSL과 일치해야 함
	struct Light
	{
		XMFLOAT4 position;
		XMFLOAT3 direction;
		float spotAngle; // innerCone - outerCone
		XMFLOAT4 diffuseColor;
		XMFLOAT4 specularColor;
		float specularPower;
		float outerConeCos;
		float innerConeCos;
		int lightType; // 0: Directional, 1: Point, 2: Spot
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		Light directionalLight;
		Light spotLights[3]; // 셰이더의 NUM_SPOT_LIGHTS와 일치
	};

	struct TimeBufferType
	{
		float time;
		float padding1, padding2, padding3; // 16바이트 정렬
	};

public:
	OceanShaderClass();
	OceanShaderClass(const OceanShaderClass&) = delete;
	OceanShaderClass& operator=(const OceanShaderClass&) = delete;
	~OceanShaderClass();

	bool Initialize(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix, const vector<ID3D11ShaderResourceView*>& textures,
		const vector<LightClass*>& lights, CameraClass* camera, float time);

private:
	bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* shaderFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
		XMMATRIX projectionMatrix, const vector<ID3D11ShaderResourceView*>& textures,
		const vector<LightClass*>& lights, CameraClass* camera, float time);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_timeBuffer;
};

#endif