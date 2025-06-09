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
    // ���� ��Ű�׵� ��ġ�� ����� ������ ���� (0���� �ʱ�ȭ)
    float4 skinnedPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 skinnedNormal = float3(0.0f, 0.0f, 0.0f);

    // 1. �� ���� ���� ��ȯ�� ��ġ�� ����� ����ϰ�, ����ġ�� ���Ͽ� ����
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.x]) * input.weights.x;
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.y]) * input.weights.y;
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.z]) * input.weights.z;
    skinnedPosition += mul(input.position, finalBones[input.boneIDs.w]) * input.weights.w;

    // (����� �̵�(translation)�� ������ ���� �ʵ��� w=0���� ��ȯ �� ���)
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.x]).xyz * input.weights.x;
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.y]).xyz * input.weights.y;
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.z]).xyz * input.weights.z;
    skinnedNormal += mul(float4(input.normal, 0.0f), finalBones[input.boneIDs.w]).xyz * input.weights.w;
    // ---- ��Ű�� ��� ���� ----

    // ���� ��ġ�� w ������Ʈ�� 1�̾�� �ϹǷ�, ��� �� �ٽ� ����
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
    float4 finalColor = float4(0.01f, 0.01f, 0.01f, 1.0f);

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