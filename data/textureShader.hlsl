// GLOBALS //
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Texture and Sampler resources
Texture2D shaderTexture;
SamplerState SampleType;


// TYPEDEFS //
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL; 
};

// Pixel Shader의 입력 구조체.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;

};


// Vertex Shader
PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;

	// 월드, 뷰, 프로젝션 행렬을 사용하여 정점 위치 변환
    input.position.w = 1.0f;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// 픽셀 셰이더로 텍스처 좌표 전달
    output.tex = input.tex;
    
    return output;
}


// Pixel Shader
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	// 텍스처에서 픽셀 색상 샘플링
    return shaderTexture.Sample(SampleType, input.tex);
}