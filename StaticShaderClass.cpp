////////////////////////////////////////////////////////////////////////////////
// Filename: staticshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "staticshaderclass.h" // 헤더 파일 이름 변경


StaticShaderClass::StaticShaderClass()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_sampleState = nullptr;
	m_matrixBuffer = nullptr;
	m_lightBuffer = nullptr;
}

StaticShaderClass::StaticShaderClass(const StaticShaderClass& other) { }
StaticShaderClass::~StaticShaderClass() { }

bool StaticShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{

	return InitializeShader(device, hwnd, L"./data/staticLight.hlsl");
}

void StaticShaderClass::Shutdown()
{
	ShutdownShader();
}


bool StaticShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX world, XMMATRIX view, XMMATRIX projection,
	ID3D11ShaderResourceView* texture, const std::vector<LightClass*>& lights)
{
	if (!SetShaderParameters(deviceContext, world, view, projection, texture, lights))
	{
		return false;
	}
	RenderShader(deviceContext, indexCount);
	return true;
}


bool StaticShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* fileName)
{
	HRESULT result;
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer = nullptr;
	ID3D10Blob* pixelShaderBuffer = nullptr;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightColorBufferDesc;
	D3D11_BUFFER_DESC lightPositionBufferDesc;


	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	result = D3DCompileFromFile(fileName, NULL, NULL, "StaticLightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, fileName);
		}
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Compile the pixel shader code.
	result = D3DCompileFromFile(fileName, NULL, NULL, "StaticLightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if(FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, fileName);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
		}

		return false;
	}

    // Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result)) { return false; }
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result)) { return false; }

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
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


	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result)) { return false; }

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Create a texture sampler state description.
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

	// Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// --- 상수 버퍼 생성 ---
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

	// Light Buffer (b1)
	bufferDesc.ByteWidth = sizeof(LightBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result)) return false;

	return true;
}


void StaticShaderClass::ShutdownShader()
{
	if (m_lightBuffer) { m_lightBuffer->Release(); m_lightBuffer = nullptr; }
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = nullptr; }
	if (m_sampleState) { m_sampleState->Release(); m_sampleState = nullptr; }
	if (m_layout) { m_layout->Release(); m_layout = nullptr; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = nullptr; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }

	return;
}


void StaticShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

// The SetShaderParameters function now takes in lightDirection and diffuseColor as inputs.
bool StaticShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX world, XMMATRIX view, XMMATRIX projection,
	ID3D11ShaderResourceView* texture, const std::vector<LightClass*>& lights) 
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// --- Matrix Buffer 업데이트 (b0) ---
	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = world;
	dataPtr->view = view;
	dataPtr->projection = projection;
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);


	// --- Bone Buffer 업데이트 (b1) ---
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	LightBufferType* dataPtr2 = (LightBufferType*)mappedResource.pData;
	for (size_t i = 0; i < NUM_LIGHTS_STATIC; ++i)
	{
		if (i < lights.size() && lights[i] != nullptr)
		{
			dataPtr2->lights[i].diffuseColor = lights[i]->GetDiffuseColor();
			dataPtr2->lights[i].lightPosition = lights[i]->GetPosition();
		}
		else
		{
			dataPtr2->lights[i].diffuseColor = XMFLOAT4(0, 0, 0, 0);
			dataPtr2->lights[i].lightPosition = XMFLOAT4(0, 0, 0, 0);
		}
	}
	deviceContext->Unmap(m_lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &m_lightBuffer);


	// --- 텍스처 리소스 설정 (t0) ---
	deviceContext->PSSetShaderResources(0, 1, &texture);
	return true;
}


void StaticShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// LightShaderClass의 구현과 동일
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}