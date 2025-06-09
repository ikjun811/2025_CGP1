#define NUM_LIGHTS 4 

// -- 상수 버퍼 (Constant Buffers) --
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct Light
{
    float4 diffuseColor;
    float4 lightPosition;
};

cbuffer LightBuffer : register(b1)
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

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

// -- Vertex Shader --
PixelInputType StaticLightVertexShader(VertexInputType input)
{
    PixelInputType output;
   
    input.position.w = 1.0f;
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    output.worldPosition = mul(input.position, worldMatrix).xyz;

    return output;
}

// -- Pixel Shader --
float4 StaticLightPixelShader(PixelInputType input) : SV_TARGET 
{
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);
    float4 finalColor = float4(0.15f, 0.15f, 0.15f, 1.0f); // Ambient

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        float3 lightDir = normalize(lights[i].lightPosition.xyz - input.worldPosition);
        float lightIntensity = saturate(dot(input.normal, lightDir));
        finalColor += lights[i].diffuseColor * lightIntensity;
    }

    return saturate(finalColor) * textureColor;
}