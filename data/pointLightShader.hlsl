#define NUM_LIGHTS 4 

// -- 상수 버퍼 (Constant Buffers) --

// Vertex Shader에서 사용할 행렬 버퍼
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Pixel Shader에서 사용할 광원 정보 버퍼
// (하나의 버퍼로 통합하여 관리)
struct Light
{
    float4 diffuseColor;
    float4 lightPosition;
};

cbuffer LightBuffer : register(b1) // 슬롯 1번 사용
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
};

// VS -> PS로 전달할 데이터가 훨씬 간단해집니다.
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
   
    input.position.w = 1.0f;

	// 최종 스크린 좌표 계산
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// 텍스처 좌표 전달
    output.tex = input.tex;
    
	// 법선을 월드 공간으로 변환하고 정규화
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    // 정점의 월드 좌표 계산하여 전달
    output.worldPosition = mul(input.position, worldMatrix).xyz;

    return output;
}


// -- Pixel Shader --
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    // 텍스처에서 기본 색상 가져오기
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // 최종 색상을 더해갈 변수 (기본값은 검은색)
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

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