////////////////////////////////////////////////////////////////////////////////
// Filename: ocean.hlsl
////////////////////////////////////////////////////////////////////////////////

// --- 상수 버퍼 (Constant Buffers) ---

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
    Light spotLights[3]; 
};

cbuffer TimeBuffer : register(b3)
{
    float time;
    float3 padding2; // 16바이트 정렬
};


// --- 리소스 (Textures & Sampler) ---
Texture2D baseColorTexture : register(t0); // 기본 바다색
Texture2D normalMap1 : register(t1); // 첫 번째 물결 노멀맵
Texture2D normalMap2 : register(t2); // 두 번째 물결 노멀맵
SamplerState SampleType : register(s0);


// --- 데이터 구조체 (Typedefs) ---

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL; // 또는 bitangent
    float3 viewDirection : TEXCOORD1;
    float3 worldPosition : TEXCOORD2;
};


// --- 조명 계산 헬퍼 함수 ---
float4 ComputeLighting(float3 normal, float3 lightDir, float3 viewDir, Light light)
{
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // 확산광(Diffuse) 계산
    float lightIntensity = saturate(dot(normal, lightDir));
    if (lightIntensity > 0.0f)
    {
        finalColor += light.diffuseColor * lightIntensity;
        
        // 반사광(Specular) 계산
        float3 reflection = normalize(reflect(-lightDir, normal));
        float specularFactor = pow(saturate(dot(reflection, viewDir)), light.specularPower);
        finalColor += light.specularColor * specularFactor;
    }
    
    return finalColor;
}


// --- Vertex Shader ---
PixelInputType OceanVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 정점 위치를 월드, 뷰, 투영 공간으로 변환
    output.position = mul(input.position, worldMatrix);
    output.worldPosition = output.position.xyz; // 픽셀 셰이더에서 사용할 월드 좌표 저장
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // 텍스처 좌표 전달
    output.tex = input.tex;
    
    // 노멀, 탄젠트, 바이노멀(바이탄젠트) 벡터를 월드 공간으로 변환
    // 노멀 매핑을 위한 TBN 기저 벡터(basis vectors) 생성
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3) worldMatrix));
    output.binormal = normalize(cross(output.normal, output.tangent));
	
    // 뷰 방향 벡터 계산
    output.viewDirection = normalize(cameraPosition.xyz - output.worldPosition.xyz);

    return output;
}


// --- Pixel Shader ---
float4 OceanPixelShader(PixelInputType input) : SV_TARGET
{
    // 1. 텍스처 좌표 스크롤링
    // time 변수를 사용하여 두 개의 노멀 맵 UV를 서로 다른 방향과 속도로 이동
    float2 scrolledUV1 = input.tex + float2(time * 0.015f, time * 0.02f);
    float2 scrolledUV2 = input.tex - float2(time * 0.025f, time * 0.01f);

    // 2. 두 노멀 맵에서 노멀(법선) 값 샘플링
    float3 bumpNormal1 = (normalMap1.Sample(SampleType, scrolledUV1).rgb * 2.0f) - 1.0f;
    float3 bumpNormal2 = (normalMap2.Sample(SampleType, scrolledUV2).rgb * 2.0f) - 1.0f;
    
    // 3. 두 노멀 값을 더하여 최종 노멀 맵의 법선 벡터 생성
    float3 bumpNormal = bumpNormal1 + bumpNormal2;

    float3x3 tbnMatrix = float3x3(input.tangent, input.binormal, input.normal);
    float3 finalNormal = normalize(mul(bumpNormal, tbnMatrix));
    
    // 5. 기본 바다색 텍스처 샘플링
    float4 baseColor = baseColorTexture.Sample(SampleType, input.tex);

    // 6. 최종 노멀을 사용하여 조명 계산
    // 6-1. Ambient(주변광)
    float4 finalColor = ambientColor;

    // 6-2. Directional Light(방향성 광원)
    finalColor += ComputeLighting(finalNormal, -directionalLight.direction, input.viewDirection, directionalLight);
    
    // 6-3. Spot Lights(스포트라이트)
    for (int i = 0; i < 3; i++)
    {
        float3 lightVec = spotLights[i].position.xyz - input.worldPosition;
        float dist = length(lightVec);
        float3 lightDir = normalize(lightVec);
        float spotFactor = dot(-lightDir, spotLights[i].direction);
        
        if (spotFactor > spotLights[i].outerConeCos)
        {
            float spotAttenuation = saturate((spotFactor - spotLights[i].outerConeCos) / spotLights[i].spotAngle);
            float distAttenuation = 1.0f / (1.0f + 0.01f * dist + 0.001f * dist * dist); // 간단한 거리 감쇠
            float4 spotColor = ComputeLighting(finalNormal, lightDir, input.viewDirection, spotLights[i]);
            finalColor += spotColor * spotAttenuation * distAttenuation;
        }
    }
    
    // 7. 최종 색상 = (계산된 조명 색상) * (기본 바다 텍스처 색상)
    return saturate(finalColor) * baseColor;
}