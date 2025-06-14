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
	m_billboardBuffer = nullptr; // billboardBuffer 초기화 추가
	m_sampleState = nullptr;
}


BillboardShaderClass::~BillboardShaderClass()
{
}

bool BillboardShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	// 빌보드 전용 셰이더 파일을 로드하도록 수정합니다.
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
	// SetShaderParameters에 모든 인자를 전달하도록 수정합니다.
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, cameraPosition, cameraUp))
	{
		return false;
	}

	RenderShader(deviceContext, indexCount);

	return true;
}

// InitializeShader 함수가 하나의 HLSL 파일을 받도록 수정합니다.
bool BillboardShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* shaderFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// 버텍스 셰이더 컴파일
	result = D3DCompileFromFile(shaderFilename, NULL, NULL, "BillboardVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, shaderFilename); }
		else { MessageBox(hwnd, shaderFilename, L"Missing Shader File", MB_OK); }
		return false;
	}

	// 픽셀 셰이더 컴파일
	result = D3DCompileFromFile(shaderFilename, NULL, NULL, "BillboardPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, shaderFilename); }
		else { MessageBox(hwnd, shaderFilename, L"Missing Shader File", MB_OK); }
		return false;
	}

	// 셰이더 객체 생성
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result)) return false;
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result)) return false;

	// 빌보드용 정점 레이아웃 (POSITION, TEXCOORD만 필요)
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

	// 상수 버퍼 설정
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Matrix Buffer (b0) 생성
	bufferDesc.ByteWidth = sizeof(MatrixBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_matrixBuffer); // <<-- 이 호출이 성공해야 합니다.
	if (FAILED(result))
	{
		return false; // 혹시 여기서 실패하고 있지는 않은지 확인
	}

	// Billboard Buffer (b1) 생성
	bufferDesc.ByteWidth = sizeof(BillboardBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_billboardBuffer); // <<-- 이 호출이 성공해야 합니다.
	if (FAILED(result))
	{
		return false;
	}

	// 샘플러 상태 생성
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

// SetShaderParameters 함수가 모든 인자를 받도록 수정합니다.
bool BillboardShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
	const XMFLOAT3& cameraPosition, const XMFLOAT3& cameraUp)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// --- Matrix Buffer (b0) 업데이트 ---
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	MatrixBufferType* dataPtrMat = (MatrixBufferType*)mappedResource.pData;
	dataPtrMat->world = XMMatrixTranspose(worldMatrix);
	dataPtrMat->view = XMMatrixTranspose(viewMatrix);
	dataPtrMat->projection = XMMatrixTranspose(projectionMatrix);
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// --- Billboard Buffer (b1) 업데이트 ---
	result = deviceContext->Map(m_billboardBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	BillboardBufferType* dataPtrBill = (BillboardBufferType*)mappedResource.pData;
	dataPtrBill->cameraPosition = cameraPosition;
	dataPtrBill->cameraUp = cameraUp;
	deviceContext->Unmap(m_billboardBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_billboardBuffer);

	// --- 텍스처 리소스 설정 ---
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