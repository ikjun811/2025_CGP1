////////////////////////////////////////////////////////////////////////////////
// Filename: pointLightShader.hlsl (Skeletal Animation Enabled)
////////////////////////////////////////////////////////////////////////////////
#define NUM_LIGHTS 4 
#define MAX_BONES 256 


// -- ��� ���� (Constant Buffers) --

// Vertex Shader���� ����� ��� ����
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer BoneBuffer : register(b1) // ���� 1�� ���
{
    matrix finalBones[MAX_BONES];
};

// Pixel Shader���� ����� ���� ���� ����
struct Light
{
    float4 diffuseColor;
    float4 lightPosition;
};

cbuffer LightBuffer : register(b2) // ���� 2�� ��� 
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
    uint4 boneIDs : BONEIDS0; // �� ID (������)
    float4 weights : WEIGHTS0; // �� ����ġ (�Ǽ���)
};

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
   
    // ---- ��Ű�� ��� ���� ----
    matrix skinTransform = (matrix) 0; // 0 ��ķ� �ʱ�ȭ

    // 4���� ���� ���� ��ȯ ����� ����ġ�� ���� �ջ�
    skinTransform += finalBones[input.boneIDs.x] * input.weights.x;
    skinTransform += finalBones[input.boneIDs.y] * input.weights.y;
    skinTransform += finalBones[input.boneIDs.z] * input.weights.z;
    skinTransform += finalBones[input.boneIDs.w] * input.weights.w;

    // ���� ��ġ�� ��ֿ� ��Ű��(����) ��ȯ�� ����
    float4 skinnedPosition = mul(input.position, skinTransform);
    // ����� ��ġ�� �ƴϹǷ� w=0���� �����Ͽ� ��ȯ (�̵��� ������ ���� �ʵ���)
    float3 skinnedNormal = (float3) mul(float4(input.normal, 0.0f), skinTransform);
    // ---- ��Ű�� ��� ���� ----

    skinnedPosition.w = 1.0f;

	// ���� ��ũ�� ��ǥ ��� (��Ű�׵� ��ġ ���)
    output.position = mul(skinnedPosition, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// �ؽ�ó ��ǥ ����
    output.tex = input.tex;
    
	// ������ ���� �������� ��ȯ�ϰ� ����ȭ (��Ű�׵� ��� ���)
    output.normal = mul(skinnedNormal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    // ������ ���� ��ǥ ����Ͽ� ���� (��Ű�׵� ��ġ ���)
    output.worldPosition = mul(skinnedPosition, worldMatrix).xyz;

    return output;
}


// -- Pixel Shader --
// �ȼ� ���̴��� ������ �ʿ䰡 �����ϴ�. ������ �����ϰ� �۵��մϴ�.
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    // �ؽ�ó���� �⺻ ���� ��������
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    // ���� ������ ���ذ� ���� (Ambient Light�� �ʱ�ȭ�Ͽ� ��ο� ���� ���̰� ��)
    float4 finalColor = float4(0.15f, 0.15f, 0.15f, 1.0f);

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