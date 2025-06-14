
////billboard.hlsl


cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// �������� ���� �߰� ����
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


// --- Vertex Shader (������ �ٽ� ����) ---
PixelInputType BillboardVertexShader(VertexInputType input)
{
    PixelInputType output;

    // 1. ��ü�� �߽� ��ġ�� ũ��(������)�� ���� ��Ŀ��� ����
    float3 objectPosition = worldMatrix[3].xyz;
    float scaleX = length(worldMatrix[0].xyz); // ���� ����� X�� ���� ���� = ���� ������
    float scaleY = length(worldMatrix[1].xyz); // ���� ����� Y�� ���� ���� = ���� ������

    // 2. ��ü���� ī�޶� ���ϴ� ���� ���� ���
    float3 look = normalize(cameraPosition - objectPosition);
    
    // 3. ������ ���� ���
    float3 right = normalize(cross(cameraUp, look));

    // 4. ���� ���� ���� ���
    float3 up = cross(look, right);
    
    // 5. <<-- �� �κ��� �ٽ� ���� -->>
    //    ���� ��ǥ�� ������ ������ ���� ���Ͽ� ���� ũ�⸦ �ݿ�
    float4 worldPosition = float4(objectPosition, 1.0f);
    worldPosition.xyz += right * input.position.x * scaleX;
    worldPosition.xyz += up * input.position.y * scaleY;
    
    // ���� ��ũ�� ��ǥ ���
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;

    return output;
}

// --- Pixel Shader ---
float4 BillboardPixelShader(PixelInputType input) : SV_TARGET
{
    // ���� ������ ����, �ؽ�ó�� ���� ���� 0.1���� ������ �ȼ��� ���� (discard)
    float2 flippedTex = input.tex;
    flippedTex.y = 1.0f - flippedTex.y;

    float4 textureColor = shaderTexture.Sample(SampleType, flippedTex); // ������ ��ǥ�� ���ø�
    
    if (textureColor.a < 0.1)
    {
        discard;
    }
    
    return textureColor;
}