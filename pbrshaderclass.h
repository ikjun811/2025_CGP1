#pragma once
////////////////////////////////////////////////////////////////////////////////
// Filename: pbrshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _PBRSHADERCLASS_H_
#define _PBRSHADERCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

#include "lightclass.h"
#include "cameraclass.h"

using namespace std;
using namespace DirectX;

// ���̴��� ����ȭ�� ��� (StaticShaderClass�� �����ϰ� ����)
constexpr int NUM_SPOT_LIGHTS_PBR = 3;

////////////////////////////////////////////////////////////////////////////////
// Class name: PBRShaderClass
////////////////////////////////////////////////////////////////////////////////
class PBRShaderClass
{
private:
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

    // Light ���� ����ü�� StaticShaderClass�� ����
    struct Light
    {
        XMFLOAT4 position;       XMFLOAT3 direction;      float    spotAngle;
        XMFLOAT4 diffuseColor;   XMFLOAT4 specularColor;  float    specularPower;
        float    outerConeCos;   float    innerConeCos;   int      lightType;
    };

    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        Light    directionalLight;
        Light    spotLights[NUM_SPOT_LIGHTS_PBR];
    };

public:
    PBRShaderClass();
    PBRShaderClass(const PBRShaderClass&);
    ~PBRShaderClass();

    bool Initialize(ID3D11Device* device, HWND hwnd);
    void Shutdown();

    // Render �Լ��� ���� �ؽ�ó�� �޵��� ������
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection,
        const vector<ID3D11ShaderResourceView*>& textures, // vector<...>�� ����
        const vector<LightClass*>& lights,
        CameraClass* camera);

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* filename);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* blob, HWND hwnd, const WCHAR* filename);

    // SetShaderParameters �Լ��� ���� �ؽ�ó�� �޵��� ������
    bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection,
        const vector<ID3D11ShaderResourceView*>& textures, // vector<...>�� ����
        const vector<LightClass*>& lights,
        CameraClass* camera);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_cameraBuffer;
    ID3D11Buffer* m_lightBuffer;
};

#endif