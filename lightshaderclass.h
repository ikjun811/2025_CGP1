////////////////////////////////////////////////////////////////////////////////
// Filename: lightshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <fstream>
#include <vector>

#include "lightclass.h"
#include "cameraclass.h" // CameraClass를 사용하기 위해 포함

using namespace std;
using namespace DirectX;

// 셰이더와 동기화할 상수
constexpr int NUM_SPOT_LIGHTS_ANIM = 3; // 스포트라이트 개수
constexpr int MAX_BONES = 256;         // 최대 뼈 개수

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

    struct CameraBufferType
    {
        XMFLOAT3 cameraPosition;
        float padding;
    };

    // HLSL의 Light 구조체와 대응
    struct Light
    {
        XMFLOAT4 position;
        XMFLOAT3 direction;
        float    spotAngle;

        XMFLOAT4 diffuseColor;
        XMFLOAT4 specularColor;
        float    specularPower;
        float    outerConeCos;
        float    innerConeCos;
        int      lightType; // 패딩용
    };

    // HLSL의 LightBuffer와 대응
    struct LightBufferType
    {
        XMFLOAT4 ambientColor;
        Light    directionalLight;
        Light    spotLights[NUM_SPOT_LIGHTS_ANIM];
    };

public:
    LightShaderClass();
    LightShaderClass(const LightShaderClass&);
    ~LightShaderClass();

    bool Initialize(ID3D11Device* device, HWND hwnd);
    void Shutdown();
    bool Render(ID3D11DeviceContext* deviceContext, int indexCount,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection,
        ID3D11ShaderResourceView* texture,
        const vector<LightClass*>& lights,
        const vector<XMMATRIX>& boneTransforms,
        CameraClass* camera); // 카메라 포인터 추가

private:
    bool InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* filename);
    void ShutdownShader();
    void OutputShaderErrorMessage(ID3D10Blob* blob, HWND hwnd, const WCHAR* filename);

    bool SetShaderParameters(ID3D11DeviceContext* deviceContext,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection,
        ID3D11ShaderResourceView* texture,
        const vector<LightClass*>& lights,
        const vector<XMMATRIX>& boneTransforms,
        CameraClass* camera);
    void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_layout;
    ID3D11SamplerState* m_sampleState;

    // 상수 버퍼
    ID3D11Buffer* m_matrixBuffer;
    ID3D11Buffer* m_boneBuffer;
    ID3D11Buffer* m_cameraBuffer;
    ID3D11Buffer* m_lightBuffer;
};

#endif