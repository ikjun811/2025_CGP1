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

// Pixel Shader�� �Է� ����ü.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;

};


// Vertex Shader
PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;

	// ����, ��, �������� ����� ����Ͽ� ���� ��ġ ��ȯ
    input.position.w = 1.0f;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// �ȼ� ���̴��� �ؽ�ó ��ǥ ����
    output.tex = input.tex;
    
    return output;
}


// Pixel Shader
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	// �ؽ�ó���� �ȼ� ���� ���ø�
    return shaderTexture.Sample(SampleType, input.tex);
}