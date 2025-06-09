////////////////////////////////////////////////////////////////////////////////
// Filename: pbrshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pbrshaderclass.h"

PBRShaderClass::PBRShaderClass()
{
    m_vertexShader = nullptr;
    m_pixelShader = nullptr;
    m_layout = nullptr;
    m_sampleState = nullptr;
    m_matrixBuffer = nullptr;
    m_cameraBuffer = nullptr;
    m_lightBuffer = nullptr;
}

PBRShaderClass::PBRShaderClass(const PBRShaderClass& other) { }
PBRShaderClass::~PBRShaderClass() { }

bool PBRShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
    // PBR ���̴� ������ �ε�
    return InitializeShader(device, hwnd, L"./data/pbrShader.hlsl");
}

void PBRShaderClass::Shutdown()
{
    ShutdownShader();
}

bool PBRShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
    XMMATRIX world, XMMATRIX view, XMMATRIX projection,
    const vector<ID3D11ShaderResourceView*>& textures,
    const vector<LightClass*>& lights, CameraClass* camera)
{
    if (!SetShaderParameters(deviceContext, world, view, projection, textures, lights, camera))
    {
        return false;
    }
    RenderShader(deviceContext, indexCount);
    return true;
}

bool PBRShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* fileName)
{
    HRESULT result;
    ID3D10Blob* errorMessage = nullptr;
    ID3D10Blob* vertexShaderBuffer = nullptr;
    ID3D10Blob* pixelShaderBuffer = nullptr;

    // PBR ���̴��� �´� �Լ� �̸��� ����մϴ�.
    // HLSL ���Ͽ� ���ǵ� �Լ� �̸��� �ݵ�� ��ġ�ؾ� �մϴ�.
    const char* vsEntryPoint = "PBRVertexShader";
    const char* psEntryPoint = "PBRPixelShader";

    // ���ؽ� ���̴� ������
    result = D3DCompileFromFile(fileName, NULL, NULL, vsEntryPoint, "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, fileName);
        }
        else {
            MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
        }
        return false;
    }

    // �ȼ� ���̴� ������
    result = D3DCompileFromFile(fileName, NULL, NULL, psEntryPoint, "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage) {
            OutputShaderErrorMessage(errorMessage, hwnd, fileName);
        }
        else {
            MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
        }
        return false; // ����: true�� �ƴ� false�� ��ȯ�ؾ� �մϴ�.
    }

    // ���̴� ��ü ����
    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    if (FAILED(result)) return false;

    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
    if (FAILED(result)) return false;

    // ���� ���̾ƿ� ����. (���� Tangent, Bitangent �߰� ���� Ȯ��)
    D3D11_INPUT_ELEMENT_DESC polygonLayout[4]; // ����� 4��
    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    polygonLayout[2].SemanticName = "NORMAL";
    polygonLayout[2].SemanticIndex = 0;
    polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[2].InputSlot = 0;
    polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[2].InstanceDataStepRate = 0;

    polygonLayout[3].SemanticName = "TANGENT";
    polygonLayout[3].SemanticIndex = 0;
    polygonLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[3].InputSlot = 0;
    polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[3].InstanceDataStepRate = 0;

    unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);
    result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) return false;

    // ���̴� ���۴� �� �̻� �ʿ� �����Ƿ� ����
    vertexShaderBuffer->Release();
    vertexShaderBuffer = nullptr;
    pixelShaderBuffer->Release();
    pixelShaderBuffer = nullptr;

    // ���÷� ����(Sampler State) ����
    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result)) return false;

    // ��� ���� ����
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    // Matrix Buffer (b0)
    bufferDesc.ByteWidth = sizeof(MatrixBufferType);
    result = device->CreateBuffer(&bufferDesc, NULL, &m_matrixBuffer);
    if (FAILED(result)) return false;

    // Camera Buffer (b1)
    bufferDesc.ByteWidth = sizeof(CameraBufferType);
    result = device->CreateBuffer(&bufferDesc, NULL, &m_cameraBuffer);
    if (FAILED(result)) return false;

    // Light Buffer (b2)
    bufferDesc.ByteWidth = sizeof(LightBufferType);
    result = device->CreateBuffer(&bufferDesc, NULL, &m_lightBuffer);
    if (FAILED(result)) return false;

    return true;
}


void PBRShaderClass::ShutdownShader()
{
	// StaticShaderClass::ShutdownShader �� ����
	if (m_lightBuffer) { m_lightBuffer->Release(); m_lightBuffer = nullptr; }
	if (m_cameraBuffer) { m_cameraBuffer->Release(); m_cameraBuffer = nullptr; }
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = nullptr; }
	if (m_sampleState) { m_sampleState->Release(); m_sampleState = nullptr; }
	if (m_layout) { m_layout->Release(); m_layout = nullptr; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = nullptr; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }
}

void PBRShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
    if (!errorMessage) {
        MessageBox(hwnd, L"Unknown shader compilation error.", shaderFilename, MB_OK);
        return;
    }
    char* compileErrors = (char*)(errorMessage->GetBufferPointer());
    unsigned long long bufferSize = errorMessage->GetBufferSize();
    ofstream fout("pbr-shader-error.txt"); 
    for (unsigned long long i = 0; i < bufferSize; i++) {
        fout << compileErrors[i];
    }
    fout.close();
    errorMessage->Release();
    errorMessage = nullptr;
    MessageBox(hwnd, L"Error compiling PBR shader. Check pbr-shader-error.txt for message.", shaderFilename, MB_OK);
}


bool PBRShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX world, XMMATRIX view, XMMATRIX projection,
	const vector<ID3D11ShaderResourceView*>& textures,
	const vector<LightClass*>& lights, CameraClass* camera)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// --- Matrix Buffer ������Ʈ (VS�� b0) ---
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	MatrixBufferType* dataPtrMat = (MatrixBufferType*)mappedResource.pData;
	dataPtrMat->world = XMMatrixTranspose(world);
	dataPtrMat->view = XMMatrixTranspose(view);
	dataPtrMat->projection = XMMatrixTranspose(projection);
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);



	// --- Camera Buffer ������Ʈ (VS/PS�� b1) ---
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	CameraBufferType* dataPtrCam = (CameraBufferType*)mappedResource.pData;
	dataPtrCam->cameraPosition = camera->GetPosition();
	dataPtrCam->padding = 0.0f;
	deviceContext->Unmap(m_cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_cameraBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &m_cameraBuffer);

	// --- Light Buffer ������Ʈ (PS�� b2) ---
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	LightBufferType* dataPtrLight = (LightBufferType*)mappedResource.pData;
	dataPtrLight->ambientColor = XMFLOAT4(0.1f, 0.1f, 0.15f, 1.0f);

	int spotLightCount = 0;
	bool directionalFound = false;
	for (const auto& light : lights)
	{
		if (light->GetLightType() == LightType::Directional && !directionalFound)
		{
			dataPtrLight->directionalLight.direction = light->GetDirection();
			dataPtrLight->directionalLight.diffuseColor = light->GetDiffuseColor();
			dataPtrLight->directionalLight.specularColor = light->GetSpecularColor();
			dataPtrLight->directionalLight.specularPower = light->GetSpecularPower();
			directionalFound = true;
		}
        else if (light->GetLightType() == LightType::Spot && spotLightCount < NUM_SPOT_LIGHTS_PBR)
		{
			dataPtrLight->spotLights[spotLightCount].position = light->GetPosition();
			dataPtrLight->spotLights[spotLightCount].direction = light->GetDirection();
			dataPtrLight->spotLights[spotLightCount].diffuseColor = light->GetDiffuseColor();
			dataPtrLight->spotLights[spotLightCount].specularColor = light->GetSpecularColor();
			dataPtrLight->spotLights[spotLightCount].specularPower = light->GetSpecularPower();
			dataPtrLight->spotLights[spotLightCount].innerConeCos = light->GetInnerConeAngle();
			dataPtrLight->spotLights[spotLightCount].outerConeCos = light->GetOuterConeAngle();
			dataPtrLight->spotLights[spotLightCount].spotAngle = dataPtrLight->spotLights[spotLightCount].innerConeCos - dataPtrLight->spotLights[spotLightCount].outerConeCos;
			spotLightCount++;
		}
	}
	deviceContext->Unmap(m_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(2, 1, &m_lightBuffer);

	// --- �ؽ�ó ���ҽ� ���� (�� �κ��� �ٸ�) ---
	if (!textures.empty())
	{
		// HLSL�� t0, t1, t2... �������Ϳ� ������� ���ε�
		deviceContext->PSSetShaderResources(0, static_cast<UINT>(textures.size()), textures.data());
	}

	return true;
}

void PBRShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}