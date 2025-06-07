#define NUM_LIGHTS 4 

// -- ��� ���� (Constant Buffers) --

// Vertex Shader���� ����� ��� ����
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Pixel Shader���� ����� ���� ���� ����
// (�ϳ��� ���۷� �����Ͽ� ����)
struct Light
{
    float4 diffuseColor;
    float4 lightPosition;
};

cbuffer LightBuffer : register(b1) // ���� 1�� ���
{
    Light lights[NUM_LIGHTS];
};


// -- ���ҽ� --
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


// -- ������ ����ü (Typedefs) --

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// VS -> PS�� ������ �����Ͱ� �ξ� ���������ϴ�.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1; // ���� ��ǥ�� ����
};


// -- Vertex Shader --
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
   
    input.position.w = 1.0f;

	// ���� ��ũ�� ��ǥ ���
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// �ؽ�ó ��ǥ ����
    output.tex = input.tex;
    
	// ������ ���� �������� ��ȯ�ϰ� ����ȭ
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    // ������ ���� ��ǥ ����Ͽ� ����
    output.worldPosition = mul(input.position, worldMatrix).xyz;

    return output;
}


// -- Pixel Shader --
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    // �ؽ�ó���� �⺻ ���� ��������
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // ���� ������ ���ذ� ���� (�⺻���� ������)
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    // ��� ������ ���� ���� ����� �ݺ��մϴ�.
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        // 1. ���� ���� ���� ���
        float3 lightDir = normalize(lights[i].lightPosition.xyz - input.worldPosition);

        // 2. ���� ���� ��� (����Ʈ ���� ��)
        float lightIntensity = saturate(dot(input.normal, lightDir));

        // 3. �� ������ ���� ���� �⿩�� ���
        finalColor += lights[i].diffuseColor * lightIntensity;
    }

    // ��� ������ �⿩���� ��ģ �� �ؽ�ó ����� ���ϰ�, ������ 0~1�� �����մϴ�.
    return saturate(finalColor) * textureColor;
}