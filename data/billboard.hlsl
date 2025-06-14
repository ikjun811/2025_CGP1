
////billboard.hlsl


cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 빌보딩을 위한 추가 정보
cbuffer BillboardBuffer : register(b1)
{
    float3 cameraPosition;
    float3 cameraUp;
};

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


// --- Vertex Shader (빌보딩 핵심 로직) ---
PixelInputType BillboardVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 1. 객체의 중심 위치와 크기(스케일)를 월드 행렬에서 추출
    float3 objectPosition = worldMatrix[3].xyz;
    float scaleX = length(worldMatrix[0].xyz); // 월드 행렬의 X축 벡터 길이 = 가로 스케일
    float scaleY = length(worldMatrix[1].xyz); // 월드 행렬의 Y축 벡터 길이 = 세로 스케일

    // 2. 객체에서 카메라를 향하는 방향 벡터 계산
    float3 look = normalize(cameraPosition - objectPosition);
    
    // 3. 오른쪽 벡터 계산
    float3 right = normalize(cross(cameraUp, look));

    // 4. 실제 상향 벡터 계산
    float3 up = cross(look, right);
    
    // 5. <<-- 이 부분이 핵심 수정 -->>
    //    로컬 좌표에 추출한 스케일 값을 곱하여 최종 크기를 반영
    float4 worldPosition = float4(objectPosition, 1.0f);
    worldPosition.xyz += right * input.position.x * scaleX;
    worldPosition.xyz += up * input.position.y * scaleY;
    
    // 최종 스크린 좌표 계산
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

    return output;
}

// --- Pixel Shader ---
float4 BillboardPixelShader(PixelInputType input) : SV_TARGET
{
    // 알파 블렌딩을 위해, 텍스처의 알파 값이 0.1보다 작으면 픽셀을 버림 (discard)
    float2 flippedTex = input.tex;
    flippedTex.y = 1.0f - flippedTex.y;

    float4 textureColor = shaderTexture.Sample(SampleType, flippedTex); // 뒤집힌 좌표로 샘플링
    
    if (textureColor.a < 0.1)
    {
        discard;
    }
    
    return textureColor;
}