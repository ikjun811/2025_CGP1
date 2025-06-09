//staticLight.hlsl

#define NUM_SPOT_LIGHTS 3

// -- 상수 버퍼 (Constant Buffers) --
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
    float3 cameraPosition;
    float padding;
};
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

cbuffer LightBuffer : register(b2)
{
    float4 ambientColor;
    Light directionalLight;
    Light spotLights[NUM_SPOT_LIGHTS];
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
PixelInputType StaticLightVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.worldPosition = output.position.xyz;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.viewDirection = normalize(cameraPosition - output.worldPosition);

    return output;
}

// -- Pixel Shader --
float4 StaticLightPixelShader(PixelInputType input) : SV_TARGET
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