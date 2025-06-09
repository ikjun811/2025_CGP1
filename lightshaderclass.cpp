////////////////////////////////////////////////////////////////////////////////
// Filename: lightshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "lightshaderclass.h"
#include <vector>


LightShaderClass::LightShaderClass()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_sampleState = nullptr;
	m_matrixBuffer = nullptr;
	m_boneBuffer = nullptr;
	m_cameraBuffer = nullptr;
	m_lightBuffer = nullptr;
}

LightShaderClass::LightShaderClass(const LightShaderClass& other)
{
}


LightShaderClass::~LightShaderClass()
{
}

// The new HLSL shader files are used as input to initialize the light shader.
bool LightShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	// 퐁 조명 셰이더 파일로 변경 (애니메이션 지원 버전)
	return InitializeShader(device, hwnd, L"./data/pointLightShader.hlsl");
}



void LightShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}

bool LightShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX world, XMMATRIX view, XMMATRIX projection,
	ID3D11ShaderResourceView* texture, const vector<LightClass*>& lights,
	const vector<XMMATRIX>& boneTransforms, CameraClass* camera)
{
	if (!SetShaderParameters(deviceContext, world, view, projection, texture, lights, boneTransforms, camera))
	{
		return false;
	}

	RenderShader(deviceContext, indexCount);
	return true;
}


bool LightShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* fileName)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[5];
	unsigned int numElements;
    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightColorBufferDesc;
	D3D11_BUFFER_DESC lightPositionBufferDesc;


	// Initialize the pointers this function will use to null.



    // Compile the vertex shader code.
	result = D3DCompileFromFile(fileName, NULL, NULL, "LightVertexShader", "vs_5_0", 
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
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
	result = D3DCompileFromFile(fileName, NULL, NULL, "LightPixelShader", "ps_5_0", 
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
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
	if(FAILED(result))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
	{
		return false;
	}

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

	polygonLayout[3].SemanticName = "BONEIDS";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_UINT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	polygonLayout[4].SemanticName = "WEIGHTS";
	polygonLayout[4].SemanticIndex = 0;
	polygonLayout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[4].InputSlot = 0;
	polygonLayout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[4].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

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

	

	// --- 상수 버퍼 생성 ---
	D3D11_BUFFER_DESC bufferDesc;

	// Matrix Buffer (b0)
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Matrix Buffer (b0)
	bufferDesc.ByteWidth = sizeof(MatrixBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result)) return false;

	// Bone Buffer (b1)
	bufferDesc.ByteWidth = sizeof(BoneBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_boneBuffer);
	if (FAILED(result)) return false;

	// Camera Buffer (b2)
	bufferDesc.ByteWidth = sizeof(CameraBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_cameraBuffer);
	if (FAILED(result)) return false;

	// Light Buffer (b3)
	bufferDesc.ByteWidth = sizeof(LightBufferType);
	result = device->CreateBuffer(&bufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result)) return false;

	return true;
}


void LightShaderClass::ShutdownShader()
{
	if (m_lightBuffer) { m_lightBuffer->Release(); m_lightBuffer = nullptr; }
	if (m_cameraBuffer) { m_cameraBuffer->Release(); m_cameraBuffer = nullptr; }
	if (m_boneBuffer) { m_boneBuffer->Release(); m_boneBuffer = nullptr; }
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = nullptr; }
	if (m_sampleState) { m_sampleState->Release(); m_sampleState = nullptr; }
	if (m_layout) { m_layout->Release(); m_layout = nullptr; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = nullptr; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = nullptr; }

	return;
}


void LightShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename)
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

bool LightShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX world, XMMATRIX view, XMMATRIX projection,
	ID3D11ShaderResourceView* texture,
	const vector<LightClass*>& lights,
	const vector<XMMATRIX>& boneTransforms,
	CameraClass* camera)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// --- Matrix Buffer 업데이트 (VS의 b0) ---
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	MatrixBufferType* dataPtrMat = (MatrixBufferType*)mappedResource.pData;
	dataPtrMat->world = XMMatrixTranspose(world);
	dataPtrMat->view = XMMatrixTranspose(view);
	dataPtrMat->projection = XMMatrixTranspose(projection);
	deviceContext->Unmap(m_matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	// --- Bone Buffer 업데이트 (VS의 b1) ---
	result = deviceContext->Map(m_boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	BoneBufferType* dataPtrBones = (BoneBufferType*)mappedResource.pData;
	for (size_t i = 0; i < boneTransforms.size(); ++i)
	{
		if (i < MAX_BONES)
		{
			dataPtrBones->finalBones[i] = XMMatrixTranspose(boneTransforms[i]);
		}
	}
	deviceContext->Unmap(m_boneBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &m_boneBuffer);

	// --- Camera Buffer 업데이트 (VS/PS의 b2) ---
	result = deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;
	CameraBufferType* dataPtrCam = (CameraBufferType*)mappedResource.pData;
	dataPtrCam->cameraPosition = camera->GetPosition();
	dataPtrCam->padding = 0.0f;
	deviceContext->Unmap(m_cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(2, 1, &m_cameraBuffer);
	deviceContext->PSSetConstantBuffers(2, 1, &m_cameraBuffer);

	// --- Light Buffer 업데이트 (PS의 b3) ---
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
		else if (light->GetLightType() == LightType::Spot && spotLightCount < NUM_SPOT_LIGHTS_ANIM)
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
	deviceContext->PSSetConstantBuffers(3, 1, &m_lightBuffer);

	// --- 텍스처 리소스 설정 ---
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}

void LightShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}