////////////////////////////////////////////////////////////////////////////////
// Filename: pointLightShader.hlsl (Skeletal Animation Enabled)
////////////////////////////////////////////////////////////////////////////////
#define NUM_SPOT_LIGHTS 3
#define MAX_BONES 256


// -- ��� ���� (Constant Buffers) --

// Vertex Shader���� ����� ��� ����
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer BoneBuffer : register(b1) // �ִϸ��̼ǿ�
{
    matrix finalBones[MAX_BONES];
};

cbuffer CameraBuffer : register(b2)
{
    float3 cameraPosition;
    float padding;
};

// Pixel Shader���� ����� ���� ���� ����
struct Light
{
    float4 position;
    float3 direction;
    float spotAngle;
    float4 diffuseColor;
    float4 specularColor;
    float specularPower;
    float outerConeCos;
    float innerConeCos;
    int lightType;
};

cbuffer LightBuffer : register(b3)
{
    float4 ambientColor;
    Light directionalLight;
    Light spotLights[NUM_SPOT_LIGHTS];
};

// -- ���ҽ� --
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


// -- ������ ����ü (Typedefs) --

struct VertexInputType // �ִϸ��̼� ����
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    uint4 boneIDs : BONEIDS0;
    float4 weights : WEIGHTS0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 worldPosition : TEXCOORD2;
};

float4 ComputeLighting(float3 normal, float3 lightDir, float3 viewDir, Light light)
{
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float intensity = saturate(dot(normal, lightDir));
    if (intensity > 0.0f)
    {
        finalColor += light.diffuseColor * intensity;
        float3 reflection = normalize(reflect(-lightDir, normal));
        float specularFactor = pow(saturate(dot(reflection, viewDir)), light.specularPower);
        finalColor += light.specularColor * specularFactor;
    }
    return finalColor;
}

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
    output.worldPosition = output.position.xyz;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    output.normal = normalize(mul(skinnedNormal, (float3x3) worldMatrix));
    output.viewDirection = normalize(cameraPosition - output.worldPosition);


    return output;
}



// -- Pixel Shader --
// �ȼ� ���̴��� ������ �ʿ䰡 �����ϴ�. ������ �����ϰ� �۵��մϴ�.
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    
    float4 finalColor = ambientColor;
    finalColor += ComputeLighting(input.normal, -directionalLight.direction, input.viewDirection, directionalLight);
    
    for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
    {
        float3 lightVec = spotLights[i].position.xyz - input.worldPosition;
        float dist = length(lightVec);
        float3 lightDir = normalize(lightVec);
        float spotFactor = dot(-lightDir, spotLights[i].direction);
        
        if (spotFactor > spotLights[i].outerConeCos)
        {
            float spotAttenuation = saturate((spotFactor - spotLights[i].outerConeCos) / spotLights[i].spotAngle);
            float distAttenuation = 1.0f / (1.0f + 0.01f * dist + 0.001f * dist * dist);
            float4 spotColor = ComputeLighting(input.normal, lightDir, input.viewDirection, spotLights[i]);
            finalColor += spotColor * spotAttenuation * distAttenuation;
        }
    }
    
    return saturate(finalColor) * textureColor;
}