////////////////////////////////////////////////////////////////////////////////
// Filename: pbrShader.hlsl
// Description: Physically Based Rendering shader with multiple textures.
////////////////////////////////////////////////////////////////////////////////


// --- 상수 (C++ 클래스와 일치) ---
#define NUM_SPOT_LIGHTS 3

// --- 상수 버퍼  ---
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
    
    float4 diffuseColor; // PBR에서는 LightColor로 사용
    float4 specularColor; // 사용되지 않지만, 패딩 및 호환성을 위해 유지
    float specularPower; // 사용되지 않지만, 패딩 및 호환성을 위해 유지
    float outerConeCos;
    float innerConeCos;
    int lightType;
};

cbuffer LightBuffer : register(b2)
{
    float4 ambientColor; // PBR에서는 Image-Based Lighting(IBL)의 Irradiance로 사용됨
    Light directionalLight;
    Light spotLights[NUM_SPOT_LIGHTS];
};


// --- 리소스 (텍스처 레지스터) ---
// ModelClass에서 전달하는 순서와 정확히 일치해야 함
Texture2D albedoMap : register(t0); // = Diffuse Map
Texture2D normalMap : register(t1);
Texture2D metallicMap : register(t2); // Metallic/Roughness/AO 등을 한 텍스처에 담는 경우가 많음
Texture2D roughnessMap : register(t3); // 여기서는 분리된 맵으로 가정
Texture2D aoMap : register(t4);
Texture2D emissiveMap : register(t5); // Emissive 맵 추가

SamplerState SampleType : register(s0);


// --- 데이터 구조체 ---
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT; // 탄젠트 입력 추가
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


// --- PBR 조명 계산 함수 (Cook-Torrance BRDF) ---

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
// HLSL 파일과 일치하도록 함수 이름 지정
PixelInputType PBRVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    // 월드 변환
    output.worldPosition = mul(input.position, worldMatrix).xyz;
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3) worldMatrix));
    // 비탄젠트(Bitangent)는 외적(cross product)으로 계산
    output.bitangent = normalize(cross(output.normal, output.tangent));
    
    // 최종 스크린 좌표
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

    return output;
}


// --- Pixel Shader ---
// HLSL 파일과 일치하도록 함수 이름 지정
float4 PBRPixelShader(PixelInputType input) : SV_TARGET
{
    // 1. 텍스처에서 재질 속성 샘플링
    float3 albedo = albedoMap.Sample(SampleType, input.tex).rgb;
    float metallic = metallicMap.Sample(SampleType, input.tex).r;
    float roughness = roughnessMap.Sample(SampleType, input.tex).r;
    float ao = aoMap.Sample(SampleType, input.tex).r;
    float3 emissive = emissiveMap.Sample(SampleType, input.tex).rgb;

    // 노멀 맵에서 노멀 벡터 가져오기
    float3 tangentNormal = normalMap.Sample(SampleType, input.tex).rgb * 2.0 - 1.0;

    // TBN 행렬을 이용해 노멀을 월드 공간으로 변환
    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
    float3 N = normalize(mul(tangentNormal, TBN));

    // 2. 조명 계산에 필요한 벡터들 정의
    float3 V = normalize(cameraPosition - input.worldPosition); // View direction
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic); // 비금속(0.04)과 금속(albedo) 사이의 반사율 보간

    // 3. 조명 계산 (모든 빛에 대한 기여도 누적)
    float3 Lo = float3(0.0, 0.0, 0.0);

    // 전역 방향성 조명
    {
        float3 L = normalize(-directionalLight.direction);
        float3 H = normalize(V + L);
        float3 radiance = directionalLight.diffuseColor.rgb * 2.0; // 밝기 조절
        
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 kD = (1.0 - F) * (1.0 - metallic);
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0으로 나누는 것 방지
        float3 specular = numerator / denominator;
            
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // 스포트라이트 (모든 스포트라이트에 대해 반복)
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i)
    {
        float3 lightVec = spotLights[i].position.xyz - input.worldPosition;
        float dist = length(lightVec);
        float3 L = normalize(lightVec);
        float3 H = normalize(V + L);
        float3 radiance = spotLights[i].diffuseColor.rgb;
        
        // 스포트라이트 감쇠
        float spotFactor = dot(-L, spotLights[i].direction);
        if (spotFactor > spotLights[i].outerConeCos)
        {
            float spotAttenuation = saturate((spotFactor - spotLights[i].outerConeCos) / spotLights[i].spotAngle);
            float distAttenuation = 1.0 / (dist * dist + 1.0); // 거리 감쇠
            radiance *= spotAttenuation * distAttenuation * 100.0; // 밝기 조절
            
            // PBR 계산
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

    // 4. 최종 색상 조합
    // Ambient Light (IBL을 사용하지 않을 경우, AO와 ambientColor로 근사)
    float3 ambient = ambientColor.rgb * albedo * ao;
    float3 color = ambient + Lo;
    
    // 방출광 추가
    color += emissive;

    // HDR to LDR (Tone Mapping) 및 감마 보정
    color = color / (color + float3(1.0, 1.0, 1.0)); // Reinhard Tone Mapping
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2)); // Gamma Correction

    return float4(color, 1.0);
}
