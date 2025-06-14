////////////////////////////////////////////////////////////////////////////////
// Filename: BillboardShaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "BillboardShaderclass.h"

BillboardShaderClass::BillboardShaderClass()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_billboardBuffer = nullptr; // billboardBuffer �ʱ�ȭ �߰�
	m_sampleState = nullptr;
}


BillboardShaderClass::~BillboardShaderClass()
{
}

bool BillboardShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	// ������ ���� ���̴� ������ �ε��ϵ��� �����մϴ�.
	return InitializeShader(device, hwnd, L"./data/billboard.hlsl");
}

void BillboardShaderClass::Shutdown()
{
	ShutdownShader();
}

bool BillboardShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
	const XMFLOAT3& cameraPosition, const XMFLOAT3& cameraUp)
{
	// SetShaderParameters�� ��� ���ڸ� �����ϵ��� �����մϴ�.
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, cameraPosition, cameraUp))
	{
		return false;
	}

	RenderShader(deviceContext, indexCount);

	return true;
}

// InitializeShader �Լ��� �ϳ��� HLSL ������ �޵��� �����մϴ�.
bool BillboardShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* shaderFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// ���ؽ� ���̴� ������
	result = D3DCompileFromFile(shaderFilename, NULL, NULL, "BillboardVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, shaderFilename); }
		else { MessageBox(hwnd, shaderFilename, L"Missing Shader File", MB_OK); }
		return false;
	}

	// �ȼ� ���̴� ������
	result = D3DCompileFromFile(shaderFilename, NULL, NULL, "BillboardPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, shaderFilename); }
		else { MessageBox(hwnd, shaderFilename, L"Missing Shader File", MB_OK); }
		return false;
	}

	// ���̴� ��ü ����
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result)) return false;
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result)) return false;

	// ������� ���� ���̾ƿ� (POSITION, TEXCOORD�� �ʿ�)
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
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

	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result)) return false;

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// ��� ���� ����
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Matrix Buffer (b0) ����
	bufferDesc.ByteWidth = sizeof(MatrixBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_matrixBuffer); // <<-- �� ȣ���� �����ؾ� �մϴ�.
	if (FAILED(result))
	{
		return false; // Ȥ�� ���⼭ �����ϰ� ������ ������ Ȯ��
	}

	// Billboard Buffer (b1) ����
	bufferDesc.ByteWidth = sizeof(BillboardBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_billboardBuffer); // <<-- �� ȣ���� �����ؾ� �մϴ�.
	if (FAILED(result))
	{
		return false;
	}

	// ���÷� ���� ����
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result)) return false;

	return true;
}

void BillboardShaderClass::ShutdownShader()
{
	if (m_sampleState) { m_sampleState->Release(); m_sampleState = nullptr; }
	if (m_billboardBuffer) { m_billboardBuffer->Release(); m_billboardBuffer = nullptr; }
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = nullptr; }
	if (m_layout) { m_layout->Release(); m_layout = nullptr; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = nullptr; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }
}

void BillboardShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize;
	ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();
	fout.open("shader-error.txt");
	for (unsigned long long i = 0; i < bufferSize; i++) { fout << compileErrors[i]; }
	fout.close();

	errorMessage->Release();
	errorMessage = nullptr;

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

// SetShaderParameters �Լ��� ��� ���ڸ� �޵��� �����մϴ�.
bool BillboardShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
	const XMFLOAT3& cameraPosition, const XMFLOAT3& cameraUp)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// --- Matrix Buffer (b0) ������Ʈ ---
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	MatrixBufferType* dataPtrMat = (MatrixBufferType*)mappedResource.pData;
	dataPtrMat->world = XMMatrixTranspose(worldMatrix);
	dataPtrMat->view = XMMatrixTranspose(viewMatrix);
	dataPtrMat->projection = XMMatrixTranspose(projectionMatrix);
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// --- Billboard Buffer (b1) ������Ʈ ---
	result = deviceContext->Map(m_billboardBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	BillboardBufferType* dataPtrBill = (BillboardBufferType*)mappedResource.pData;
	dataPtrBill->cameraPosition = cameraPosition;
	dataPtrBill->cameraUp = cameraUp;
	deviceContext->Unmap(m_billboardBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_billboardBuffer);

	// --- �ؽ�ó ���ҽ� ���� ---
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

void BillboardShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}