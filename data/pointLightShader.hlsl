////////////////////////////////////////////////////////////////////////////////
// Filename: pointLightShader.hlsl (Skeletal Animation Enabled)
////////////////////////////////////////////////////////////////////////////////
#define NUM_LIGHTS 4 
#define MAX_BONES 256 


// -- 상수 버퍼 (Constant Buffers) --

// Vertex Shader에서 사용할 행렬 버퍼
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer BoneBuffer : register(b1) // 슬롯 1번 사용
{
    matrix finalBones[MAX_BONES];
};

// Pixel Shader에서 사용할 광원 정보 버퍼
struct Light
{
    float4 diffuseColor;
    float4 lightPosition;
};

cbuffer LightBuffer : register(b2) // 슬롯 2번 사용 
{
    Light lights[NUM_LIGHTS];
};


// -- 리소스 --
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


// -- 데이터 구조체 (Typedefs) --

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    uint4 boneIDs : BONEIDS0; // 뼈 ID (정수형)
    float4 weights : WEIGHTS0; // 뼈 가중치 (실수형)
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1; // 월드 좌표만 전달
};


// -- Vertex Shader --
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
   
    // ---- 스키닝 계산 시작 ----
    // 최종 스키닝된 위치와 노멀을 저장할 변수 (0으로 초기화)
    float4 skinnedPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 skinnedNormal = float3(0.0f, 0.0f, 0.0f);

    // 1. 각 뼈에 의해 변환된 위치와 노멀을 계산하고, 가중치를 곱하여 누적
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.x]) * input.weights.x;
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.y]) * input.weights.y;
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.z]) * input.weights.z;
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.w]) * input.weights.w;

    // (노멀은 이동(translation)에 영향을 받지 않도록 w=0으로 변환 후 계산)
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.x]).xyz * input.weights.x;
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.y]).xyz * input.weights.y;
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.z]).xyz * input.weights.z;
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.w]).xyz * input.weights.w;
    // ---- 스키닝 계산 종료 ----

    // 원본 위치의 w 컴포넌트는 1이어야 하므로, 계산 후 다시 설정
    skinnedPosition.w = 1.0f;

    // 최종 스크린 좌표 계산 (스키닝된 위치 사용)
    output.position = mul(skinnedPosition, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 텍스처 좌표 전달
    output.tex = input.tex;
    
    // 법선을 월드 공간으로 변환하고 정규화 (스키닝된 노멀 사용)
    output.normal = mul(skinnedNormal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    output.worldPosition = mul(skinnedPosition, worldMatrix).xyz;


    return output;
}



// -- Pixel Shader --
// 픽셀 셰이더는 수정할 필요가 없습니다. 이전과 동일하게 작동합니다.
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    // 텍스처에서 기본 색상 가져오기
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // 최종 색상을 더해갈 변수 (Ambient Light로 초기화하여 어두운 곳도 보이게 함)
    float4 finalColor = float4(0.01f, 0.01f, 0.01f, 1.0f);

    // 모든 광원에 대해 조명 계산을 반복합니다.
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        // 1. 광원 방향 벡터 계산
        float3 lightDir = normalize(lights[i].lightPosition.xyz - input.worldPosition);

        // 2. 빛의 세기 계산 (램버트 조명 모델)
        float lightIntensity = saturate(dot(input.normal, lightDir));

        // 3. 이 광원에 의한 색상 기여도 계산
        finalColor += lights[i].diffuseColor * lightIntensity;
    }

    // 모든 광원의 기여도를 합친 후 텍스처 색상과 곱하고, 범위를 0~1로 제한합니다.
    return saturate(finalColor) * textureColor;
}