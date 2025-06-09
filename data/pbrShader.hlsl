////////////////////////////////////////////////////////////////////////////////
// Filename: pbrShader.hlsl
// Description: Physically Based Rendering shader with multiple textures.
////////////////////////////////////////////////////////////////////////////////


// --- ��� (C++ Ŭ������ ��ġ) ---
#define NUM_SPOT_LIGHTS 3

// --- ��� ����  ---
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
    
    float4 diffuseColor; // PBR������ LightColor�� ���
    float4 specularColor; // ������ ������, �е� �� ȣȯ���� ���� ����
    float specularPower; // ������ ������, �е� �� ȣȯ���� ���� ����
    float outerConeCos;
    float innerConeCos;
    int lightType;
};

cbuffer LightBuffer : register(b2)
{
    float4 ambientColor; // PBR������ Image-Based Lighting(IBL)�� Irradiance�� ����
    Light directionalLight;
    Light spotLights[NUM_SPOT_LIGHTS];
};


// --- ���ҽ� (�ؽ�ó ��������) ---
// ModelClass���� �����ϴ� ������ ��Ȯ�� ��ġ�ؾ� ��
Texture2D albedoMap : register(t0); // = Diffuse Map
Texture2D normalMap : register(t1);
Texture2D metallicMap : register(t2); // Metallic/Roughness/AO ���� �� �ؽ�ó�� ��� ��찡 ����
Texture2D roughnessMap : register(t3); // ���⼭�� �и��� ������ ����
Texture2D aoMap : register(t4);
Texture2D emissiveMap : register(t5); // Emissive �� �߰�

SamplerState SampleType : register(s0);


// --- ������ ����ü ---
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT; // ź��Ʈ �Է� �߰�
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 worldPosition : TEXCOORD1;
};


// --- PBR ���� ��� �Լ� (Cook-Torrance BRDF) ---

const float PI = 3.14159265359;

// Normal Distribution Function (NDF) - Trowbridge-Reitz GGX
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel-Schlick Approximation
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


// --- Vertex Shader ---
// HLSL ���ϰ� ��ġ�ϵ��� �Լ� �̸� ����
PixelInputType PBRVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    // ���� ��ȯ
    output.worldPosition = mul(input.position, worldMatrix).xyz;
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3) worldMatrix));
    // ��ź��Ʈ(Bitangent)�� ����(cross product)���� ���
    output.bitangent = normalize(cross(output.normal, output.tangent));
    
    // ���� ��ũ�� ��ǥ
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

    return output;
}


// --- Pixel Shader ---
// HLSL ���ϰ� ��ġ�ϵ��� �Լ� �̸� ����
float4 PBRPixelShader(PixelInputType input) : SV_TARGET
{
    // 1. �ؽ�ó���� ���� �Ӽ� ���ø�
    float3 albedo = albedoMap.Sample(SampleType, input.tex).rgb;
    float metallic = metallicMap.Sample(SampleType, input.tex).r;
    float roughness = roughnessMap.Sample(SampleType, input.tex).r;
    float ao = aoMap.Sample(SampleType, input.tex).r;
    float3 emissive = emissiveMap.Sample(SampleType, input.tex).rgb;

    // ��� �ʿ��� ��� ���� ��������
    float3 tangentNormal = normalMap.Sample(SampleType, input.tex).rgb * 2.0 - 1.0;

    // TBN ����� �̿��� ����� ���� �������� ��ȯ
    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
    float3 N = normalize(mul(tangentNormal, TBN));

    // 2. ���� ��꿡 �ʿ��� ���͵� ����
    float3 V = normalize(cameraPosition - input.worldPosition); // View direction
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic); // ��ݼ�(0.04)�� �ݼ�(albedo) ������ �ݻ��� ����

    // 3. ���� ��� (��� ���� ���� �⿩�� ����)
    float3 Lo = float3(0.0, 0.0, 0.0);

    // ���� ���⼺ ����
    {
        float3 L = normalize(-directionalLight.direction);
        float3 H = normalize(V + L);
        float3 radiance = directionalLight.diffuseColor.rgb * 2.0; // ��� ����
        
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 kD = (1.0 - F) * (1.0 - metallic);
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0���� ������ �� ����
        float3 specular = numerator / denominator;
            
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ����Ʈ����Ʈ (��� ����Ʈ����Ʈ�� ���� �ݺ�)
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i)
    {
        float3 lightVec = spotLights[i].position.xyz - input.worldPosition;
        float dist = length(lightVec);
        float3 L = normalize(lightVec);
        float3 H = normalize(V + L);
        float3 radiance = spotLights[i].diffuseColor.rgb;
        
        // ����Ʈ����Ʈ ����
        float spotFactor = dot(-L, spotLights[i].direction);
        if (spotFactor > spotLights[i].outerConeCos)
        {
            float spotAttenuation = saturate((spotFactor - spotLights[i].outerConeCos) / spotLights[i].spotAngle);
            float distAttenuation = 1.0 / (dist * dist + 1.0); // �Ÿ� ����
            radiance *= spotAttenuation * distAttenuation * 100.0; // ��� ����
            
            // PBR ���
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
            
            float3 kD = (1.0 - F) * (1.0 - metallic);
            float3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
            float3 specular = numerator / denominator;
                
            float NdotL = max(dot(N, L), 0.0);
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
    }

    // 4. ���� ���� ����
    // Ambient Light (IBL�� ������� ���� ���, AO�� ambientColor�� �ٻ�)
    float3 ambient = ambientColor.rgb * albedo * ao;
    float3 color = ambient + Lo;
    
    // ���Ɽ �߰�
    color += emissive;

    // HDR to LDR (Tone Mapping) �� ���� ����
    color = color / (color + float3(1.0, 1.0, 1.0)); // Reinhard Tone Mapping
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2)); // Gamma Correction

    return float4(color, 1.0);
}
